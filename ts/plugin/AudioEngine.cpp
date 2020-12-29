#include <winsdkver.h>
#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>


#include <memory>
#include <stdexcept>

#include "AudioEngine.h"
#include "Win32Exception.h"


std::unique_ptr<AudioEngine> AudioEngine::CreateAudioEngine()
{
	IXAudio2* pXAudio2 = nullptr;
#ifdef _DEBUG
	auto hr = XAudio2Create(&pXAudio2, XAUDIO2_DEBUG_ENGINE);
#else
	auto hr = XAudio2Create(&pXAudio2);
#endif // DEBUG
	ThrowLastErrorIf(FAILED(hr), "Critical error: Unable to create the XAudio2 engine!");

#ifdef _DEBUG
	XAUDIO2_DEBUG_CONFIGURATION debug{ XAUDIO2_LOG_ERRORS, 0, true, false, true, true };
	pXAudio2->SetDebugConfiguration(&debug);
#endif // DEBUG

	return std::make_unique<AudioEngine>(pXAudio2);
}


AudioEngine::AudioEngine(IXAudio2* engine) : engine(engine), masterVoice(nullptr), X3DInstance(), listener()
{
	HRESULT hr = S_OK;
	// create master voice
	IXAudio2MasteringVoice* voice{ 0 };
	hr = engine->CreateMasteringVoice(&voice, AudioEngine::channels, AudioEngine::AudioEngine::samplesPerSec);
	ThrowLastErrorIf(FAILED(hr), "Critical error: Unable to create the XAudio2 mastering voice!");
	masterVoice.reset(voice);	
	
	DWORD dwChannelMask;
	masterVoice->GetChannelMask(&dwChannelMask);
	hr = X3DAudioInitialize(dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, X3DInstance);
	ThrowLastErrorIf(FAILED(hr), "Critical error: Unable to initialize X3DAudio!");

	listener.OrientFront.z = 1;
	listener.OrientTop.y = 1;

	fUseRedirectToLFE = (dwChannelMask & SPEAKER_LOW_FREQUENCY) != 0;
}

AudioEngine::~AudioEngine()
{
	if (engine)
	{
		// stop the engine
		engine->StopEngine();
	};
};

 AudioVoice AudioEngine::CreateVoice()
{

	Microsoft::WRL::ComPtr<IUnknown> reverb{0};
	HRESULT hr = XAudio2CreateReverb(reverb.GetAddressOf());
	ThrowLastErrorIf(FAILED(hr), "Critical error: Unable to create reverb effect!", hr);



	WAVEFORMATEX wfex{ 0 };
	wfex.wFormatTag = WAVE_FORMAT_PCM;
	wfex.nChannels = AudioEngine::channels;
	wfex.nSamplesPerSec = AudioEngine::samplesPerSec;
	wfex.wBitsPerSample = 16;
	wfex.nBlockAlign = (wfex.nChannels * wfex.wBitsPerSample) / 8;
	wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nBlockAlign;
	wfex.cbSize = 0;


	XAUDIO2_EFFECT_DESCRIPTOR effects[] { { reverb.Get(), TRUE, 1 } };
	XAUDIO2_EFFECT_CHAIN effectChain { 1, effects };
	IXAudio2SubmixVoice* r{ 0 };
	hr = engine->CreateSubmixVoice(&r, wfex.nChannels, wfex.nSamplesPerSec, 0, 0, nullptr, &effectChain);
	ThrowLastErrorIf(FAILED(hr), "Critical error: Unable to create reverb voice!", hr);
	submix_voice_ptr reverbVoice{ r };

	XAUDIO2FX_REVERB_PARAMETERS native{ 0 };
	XAUDIO2FX_REVERB_I3DL2_PARAMETERS p = PRESET_PARAMS[0];
	ReverbConvertI3DL2ToNative(&p, &native, wfex.nChannels == 8);
	hr = reverbVoice->SetEffectParameters(0, &native, sizeof(native));
	ThrowLastErrorIf(FAILED(hr), "Critical error: Unable to set reverb params!", hr);


	IXAudio2SourceVoice* source{ 0 };

	XAUDIO2_SEND_DESCRIPTOR send[]{
		{XAUDIO2_SEND_USEFILTER, masterVoice.get()},
		{XAUDIO2_SEND_USEFILTER, reverbVoice.get()}
	};
	XAUDIO2_VOICE_SENDS sendlist{ sizeof(send)/sizeof(send[0]), send };
	hr = engine->CreateSourceVoice(&source, &wfex, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &callback, &sendlist, NULL);
	ThrowLastErrorIf(FAILED(hr), "Critical error: Unable to create XAudio2 source voice!", hr);

	hr = source->Start();
	ThrowLastErrorIf(FAILED(hr), "Critical error: Unable to start XAudio2 source voice!", hr);
	return AudioVoice(source, reverbVoice.release(), this);
}



AudioVoice::AudioVoice(IXAudio2SourceVoice* voice, IXAudio2SubmixVoice* reverb, AudioEngine* engine) 
	:	voice(voice), 
		reverb(reverb), 
		engine(engine), 
		matrix(0)
{
	XAUDIO2_VOICE_DETAILS masterVoiceDetails{ 0 };
	engine->masterVoice->GetVoiceDetails(&masterVoiceDetails);

	XAUDIO2_VOICE_DETAILS voiceDetails{ 0 };
	voice->GetVoiceDetails(&voiceDetails);

	matrix.resize(masterVoiceDetails.InputChannels * voiceDetails.InputChannels);
}


void AudioVoice::SubmitBuffer(short* samples, int sampleCount, int channels)
{
	auto bufferSize = sampleCount;
	auto dataSize = bufferSize * sizeof(short);
	auto bytes = new short[bufferSize];
	if (channels > 1)
	{
		for (int i = 0; i < sampleCount; i++)
		{
			int z = 0;
			for (int j = 0; j < channels; j++)
			{
				z += samples[i*channels + j];
			}
			bytes[i] = z / channels;
		}
	}
	else
	{
		memcpy_s(bytes, dataSize, samples, dataSize);
	}
	XAUDIO2_BUFFER buf{ 0 };
	buf.pAudioData = reinterpret_cast<byte*>(bytes);
	buf.AudioBytes = dataSize;
	buf.pContext = bytes;
	auto hr = voice->SubmitSourceBuffer(&buf);
	ThrowLastErrorIf(FAILED(hr), "Critical error: Can't submit XAudio2 buffer!");
}


void AudioVoice::Update3D(const X3DAUDIO_EMITTER* pEmitter)
{
		XAUDIO2_VOICE_DETAILS masterVoiceDetails{ 0 };
		engine->masterVoice->GetVoiceDetails(&masterVoiceDetails);

		XAUDIO2_VOICE_DETAILS voiceDetails{ 0 };
		voice->GetVoiceDetails(&voiceDetails);

		X3DAUDIO_DSP_SETTINGS settings{ 0 };
		settings.SrcChannelCount = voiceDetails.InputChannels;
		settings.DstChannelCount = masterVoiceDetails.InputChannels;
		settings.pMatrixCoefficients = matrix.data();

		DWORD dwCalcFlags = X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER
			| X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_LPF_REVERB
			| X3DAUDIO_CALCULATE_REVERB;
		if (engine->fUseRedirectToLFE)
		{
			// On devices with an LFE channel, allow the mono source data
			// to be routed to the LFE destination channel.
			dwCalcFlags |= X3DAUDIO_CALCULATE_REDIRECT_TO_LFE;
		}
		UINT32 OperationID = UINT32(InterlockedIncrement(LPLONG(&engine->OperationSetCounter)));

		X3DAudioCalculate(engine->X3DInstance, &(engine->listener), pEmitter, dwCalcFlags, &settings);
		voice->SetFrequencyRatio(settings.DopplerFactor, OperationID);

		voice->SetOutputMatrix(engine->masterVoice.get(), voiceDetails.InputChannels, masterVoiceDetails.InputChannels, settings.pMatrixCoefficients, OperationID);
		XAUDIO2_FILTER_PARAMETERS FilterParametersDirect{ LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * settings.LPFDirectCoefficient), 1.0f };
		voice->SetOutputFilterParameters(engine->masterVoice.get(), &FilterParametersDirect, OperationID);

		voice->SetOutputMatrix(reverb.get(), 1, 1, &settings.ReverbLevel, OperationID);
		XAUDIO2_FILTER_PARAMETERS FilterParametersReverb{ LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * settings.LPFReverbCoefficient), 1.0f };
		voice->SetOutputFilterParameters(reverb.get(),&FilterParametersReverb, OperationID);
		engine->engine->CommitChanges(OperationID);
}


void AudioVoice::UpdateReverb(const XAUDIO2FX_REVERB_I3DL2_PARAMETERS* params)
{
	XAUDIO2FX_REVERB_PARAMETERS native;
	ReverbConvertI3DL2ToNative(params, &native);
	reverb->SetEffectParameters(0, &native, sizeof(native));
}
