#include <windows.h>
#include <mmdeviceapi.h>
#include <assert.h>
#include <endpointvolume.h>
#include "dprintf.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantFunctionResult"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

#define DLLEXPORT extern "C" __declspec(dllexport)
#define LOG_NAME "APMHeadbanana: "

#define MAX_CH 8

static int volume_channels[MAX_CH] = {-1};
static float volume;

static IMMDevice *audioDevice = NULL;
static IAudioEndpointVolume* audioEndpoint = NULL;


BOOL WINAPI DllMain(HMODULE mod, DWORD cause, void *ctx) {

    if (cause == DLL_PROCESS_DETACH){
        dprintf(LOG_NAME "Unloading\n");
        if (audioEndpoint != NULL){
            audioEndpoint->Release();
        }
        if (audioDevice != NULL){
            audioDevice->Release();
        }
        return TRUE;
    } else if (cause != DLL_PROCESS_ATTACH) {
        return TRUE;
    }
    dprintf(LOG_NAME "Loading\n");

    volume = 100;
    volume_channels[0] = 0;
    volume_channels[1] = 1;

    HRESULT hr;

    IMMDeviceEnumerator* enumerator = NULL;

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    dprintf(LOG_NAME "CoInitializeEx: %lx\n", hr);

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
    dprintf(LOG_NAME "CoCreateInstance: %lx\n", hr);
    assert(SUCCEEDED(hr));

    hr = enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &audioDevice);
    dprintf(LOG_NAME "GetDefaultAudioEndpoint: %lx\n", hr);
    assert(SUCCEEDED(hr));

    audioDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&audioEndpoint);
    dprintf(LOG_NAME "Activate: %lx\n", hr);
    assert(SUCCEEDED(hr));

    enumerator->Release();

    dprintf(LOG_NAME "Initialized\n");

    return TRUE;
}

void updateAudioClient(){
    if (audioEndpoint != NULL) {
        for (int i = 0; i < MAX_CH; i++) {
            int channel = volume_channels[i];
            if (channel >= 0) {
                dprintf(LOG_NAME "UpdateAudioClient Ch%d: %f\n", channel, volume);
                HRESULT hr = audioEndpoint->SetChannelVolumeLevelScalar(channel, volume / 100, NULL);
                if (!SUCCEEDED(hr)){
                    dprintf(LOG_NAME "UpdateAudioClient failed on Ch%d: %lx\n", channel, hr);
                }
            }
        }
    } else {
        dprintf(LOG_NAME "UpdateAudioClient failed: null\n");
    }
}

DLLEXPORT float apmHeadphoneVolumeGet() {
    dprintf(LOG_NAME "HeadphoneVolumeGet: %f\n", volume);
    return (float)volume;
}

DLLEXPORT void apmHeadphoneVolumeSet(float val){
    dprintf(LOG_NAME "HeadphoneVolumeSet: %f\n", val);
    if (val != volume) {
        volume = val;
        updateAudioClient();
    }
}

DLLEXPORT void apmHeadphoneChannelsSet(const int* channels, const int len){
    for (int i = 0; i < MAX_CH; i++){
        volume_channels[i] = -1;
    }
    for (int i = 0; i < len; i++){
        int channel = channels[i];
        dprintf(LOG_NAME "HeadphoneChannelsSet: Ch%d = %d\n", i, channel);
        volume_channels[i] = channel;

        float currentVolume;
        HRESULT hr = audioEndpoint->GetChannelVolumeLevelScalar(channel, &currentVolume);
        if (!SUCCEEDED(hr)){
            dprintf(LOG_NAME "GetChannelVolumeLevelScalar failed on Ch%d: %lx\n", channel, hr);
        } else {
            dprintf(LOG_NAME "Current volume: %f\n", currentVolume * 100);
            volume = currentVolume * 100;
        }
    }
    //updateAudioClient();
}

DLLEXPORT int apmHeadbananaVersionGet(){
    return 1;
}

#pragma clang diagnostic pop