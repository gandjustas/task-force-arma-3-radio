#pragma once
#include <windows.h>
#include <memory>
#include <vector>
#include <xaudio2.h>
#include <x3daudio.h>
#include <xaudio2fx.h>
#include <wrl\client.h>
#include <wrl\implements.h>

const int NUM_PRESETS = 30;

const XAUDIO2FX_REVERB_I3DL2_PARAMETERS PRESET_PARAMS[NUM_PRESETS] =
{
	XAUDIO2FX_I3DL2_PRESET_DEFAULT,
	XAUDIO2FX_I3DL2_PRESET_FOREST,
	XAUDIO2FX_I3DL2_PRESET_GENERIC,
	XAUDIO2FX_I3DL2_PRESET_PADDEDCELL,
	XAUDIO2FX_I3DL2_PRESET_ROOM,
	XAUDIO2FX_I3DL2_PRESET_BATHROOM,
	XAUDIO2FX_I3DL2_PRESET_LIVINGROOM,
	XAUDIO2FX_I3DL2_PRESET_STONEROOM,
	XAUDIO2FX_I3DL2_PRESET_AUDITORIUM,
	XAUDIO2FX_I3DL2_PRESET_CONCERTHALL,
	XAUDIO2FX_I3DL2_PRESET_CAVE,
	XAUDIO2FX_I3DL2_PRESET_ARENA,
	XAUDIO2FX_I3DL2_PRESET_HANGAR,
	XAUDIO2FX_I3DL2_PRESET_CARPETEDHALLWAY,
	XAUDIO2FX_I3DL2_PRESET_HALLWAY,
	XAUDIO2FX_I3DL2_PRESET_STONECORRIDOR,
	XAUDIO2FX_I3DL2_PRESET_ALLEY,
	XAUDIO2FX_I3DL2_PRESET_CITY,
	XAUDIO2FX_I3DL2_PRESET_MOUNTAINS,
	XAUDIO2FX_I3DL2_PRESET_QUARRY,
	XAUDIO2FX_I3DL2_PRESET_PLAIN,
	XAUDIO2FX_I3DL2_PRESET_PARKINGLOT,
	XAUDIO2FX_I3DL2_PRESET_SEWERPIPE,
	XAUDIO2FX_I3DL2_PRESET_UNDERWATER,
	XAUDIO2FX_I3DL2_PRESET_SMALLROOM,
	XAUDIO2FX_I3DL2_PRESET_MEDIUMROOM,
	XAUDIO2FX_I3DL2_PRESET_LARGEROOM,
	XAUDIO2FX_I3DL2_PRESET_MEDIUMHALL,
	XAUDIO2FX_I3DL2_PRESET_LARGEHALL,
	XAUDIO2FX_I3DL2_PRESET_PLATE,
};

class VoiceCallback : public IXAudio2VoiceCallback
{
public:

	void OnStreamEnd() {}

	//Unused methods are stubs
	void OnVoiceProcessingPassEnd() { }
	void OnVoiceProcessingPassStart(UINT32 SamplesRequired) {    }
	void OnBufferEnd(void* pBufferContext) {
		delete[] pBufferContext;
	}
	void OnBufferStart(void* pBufferContext) {    }
	void OnLoopEnd(void* pBufferContext) {    }
	void OnVoiceError(void* pBufferContext, HRESULT Error) { }
};


struct AudioVoiceDeleter
{
	void operator () (IXAudio2Voice* voice) {
		voice->DestroyVoice();
	}
};
using voice_ptr = std::unique_ptr<IXAudio2SourceVoice, AudioVoiceDeleter>;
using submix_voice_ptr = std::unique_ptr<IXAudio2SubmixVoice, AudioVoiceDeleter>;
using master_voice_ptr = std::unique_ptr<IXAudio2MasteringVoice, AudioVoiceDeleter>;



class AudioVoice;

class AudioEngine
{
	friend class AudioVoice;

private:
	const int channels = 1;
	const int samplesPerSec = 48000;

	VoiceCallback callback;
	Microsoft::WRL::ComPtr<IXAudio2> engine;	// the main XAudio2 engine
	master_voice_ptr masterVoice;						// a mastering voice
	X3DAUDIO_HANDLE X3DInstance;
	X3DAUDIO_LISTENER listener;

	bool fUseRedirectToLFE;
	UINT32 OperationSetCounter = 0;

	AudioEngine() = delete;
	AudioEngine(const AudioEngine&) = delete;
	AudioEngine& operator=(const AudioEngine&) = delete;
	AudioEngine(AudioEngine&& s) = delete;

public:
	explicit AudioEngine(IXAudio2*);
	// constructor and destructor
	~AudioEngine();

	AudioVoice CreateVoice();
	static std::unique_ptr<AudioEngine> CreateAudioEngine();
};

class AudioVoice
{
	friend class AudioEngine;
public:
	void SubmitBuffer(short* samples, int sampleCount, int channels);
	void Update3D(const X3DAUDIO_EMITTER* pEmitter);
	void UpdateReverb(const XAUDIO2FX_REVERB_I3DL2_PARAMETERS* params);

	AudioVoice(AudioVoice&& v) = default;
private:
	AudioEngine* engine;
	std::vector<FLOAT32> matrix;
	submix_voice_ptr reverb;
	voice_ptr voice;

	AudioVoice(IXAudio2SourceVoice* voice, IXAudio2SubmixVoice* reverb, AudioEngine* engine);
	AudioVoice() = delete;
	AudioVoice(const AudioVoice&) = delete;
	AudioVoice& operator=(const AudioVoice&) = delete;
};
