#include <winsdkver.h>
#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>
#include <Windows.h>
#include <xaudio2.h>
#include <xaudio2fx.h>
#include <x3daudio.h>

HMODULE xaudioLibrary;

typedef HRESULT(__stdcall* XAudio2CreateFunction)(IXAudio2** ppXAudio2, UINT32 Flags, XAUDIO2_PROCESSOR XAudio2Processor);

HRESULT _stdcall XAudio2Create(_Outptr_ IXAudio2** ppXAudio2, UINT32 Flags, XAUDIO2_PROCESSOR XAudio2Processor)
{
	auto proc = GetProcAddress(xaudioLibrary, "XAudio2Create");
	return ((XAudio2CreateFunction)proc)(ppXAudio2, Flags, XAudio2Processor);
}

typedef HRESULT(__stdcall* X3DAudioInitializeFunction)(UINT32 SpeakerChannelMask, FLOAT32 SpeedOfSound, X3DAUDIO_HANDLE Instance);
HRESULT _stdcall X3DAudioInitialize(UINT32 SpeakerChannelMask, FLOAT32 SpeedOfSound, X3DAUDIO_HANDLE Instance)
{
	auto proc = GetProcAddress(xaudioLibrary, "X3DAudioInitialize");
	return ((X3DAudioInitializeFunction)proc)(SpeakerChannelMask, SpeedOfSound, Instance);
}

typedef HRESULT(__stdcall* X3DAudioCalculateFunction)(const X3DAUDIO_HANDLE Instance, const X3DAUDIO_LISTENER* pListener, const X3DAUDIO_EMITTER* pEmitter, UINT32 Flags, X3DAUDIO_DSP_SETTINGS* pDSPSettings);
void _stdcall X3DAudioCalculate(const X3DAUDIO_HANDLE Instance, const X3DAUDIO_LISTENER* pListener, const X3DAUDIO_EMITTER* pEmitter, UINT32 Flags, X3DAUDIO_DSP_SETTINGS* pDSPSettings)
{
	auto proc = GetProcAddress(xaudioLibrary, "X3DAudioCalculate");
	((X3DAudioCalculateFunction)proc)(Instance, pListener, pEmitter, Flags, pDSPSettings);
}

typedef HRESULT(__stdcall* CreateAudioReverbFunction)(IUnknown** ppApo);
HRESULT _stdcall CreateAudioReverb(IUnknown** ppApo)
{
	auto proc = GetProcAddress(xaudioLibrary, "CreateAudioReverb");
	return ((CreateAudioReverbFunction)proc)(ppApo);
}



