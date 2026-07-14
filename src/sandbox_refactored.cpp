#include <iostream>
#include "portaudio.h"

int main() {
    std::cout << "Querying PortAudio Devices inside WSL..." << std::endl;
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "Pa_Initialize failed: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    int numHostApis = Pa_GetHostApiCount();
    std::cout << "Total Host APIs Found: " << numHostApis << std::endl;
    for (int i = 0; i < numHostApis; ++i) {
        const PaHostApiInfo* apiInfo = Pa_GetHostApiInfo(i);
        if (apiInfo) {
            std::cout << "  Host API [" << i << "]: " << apiInfo->name 
                      << " (type: " << apiInfo->type << ")" << std::endl;
        }
    }

    int numDevices = Pa_GetDeviceCount();
    std::cout << "Total Devices Found: " << numDevices << std::endl;

    for (int i = 0; i < numDevices; i++) {
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
        if (!deviceInfo) continue;
        
        std::cout << "Device [" << i << "]: " << deviceInfo->name << "\n"
                  << "  Host API: " << Pa_GetHostApiInfo(deviceInfo->hostApi)->name << "\n"
                  << "  Max Input Channels: " << deviceInfo->maxInputChannels << "\n"
                  << "  Max Output Channels: " << deviceInfo->maxOutputChannels << "\n"
                  << "  Default Sample Rate: " << deviceInfo->defaultSampleRate << "\n"
                  << "----------------------------------------" << std::endl;
    }

    int defaultInput = Pa_GetDefaultInputDevice();
    std::cout << "Default Input Device Index: " << defaultInput << std::endl;
    if (defaultInput != paNoDevice) {
        std::cout << "Default Input Device Name: " << Pa_GetDeviceInfo(defaultInput)->name << std::endl;
    }

    Pa_Terminate();
    return 0;
}
