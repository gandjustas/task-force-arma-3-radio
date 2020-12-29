#pragma once
#include <winsdkver.h>
#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>

#include <xaudio2redist.h>

struct XAudioLibrary
{
	HMODULE hModule;
	HRESULT (__stdcall *XAudio2Create)(IXAudio2** ppXAudio2, UINT32 Flags, XAUDIO2_PROCESSOR XAudio2Processor);
};