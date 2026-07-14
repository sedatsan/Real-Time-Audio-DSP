#include "AudioProcessor.h"
#include <iostream>

// ============================================================================
// 1. PortAudioContext Implementation (Global Initialize/Terminate)
// ============================================================================
PortAudioContext::PortAudioContext() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "Failed to initialize PortAudio: " << Pa_GetErrorText(err) << std::endl;
        // In constructor, if it fails, it's a fatal setup error.
    }
}

PortAudioContext::~PortAudioContext() {
    Pa_Terminate();
}

// ============================================================================
// 2. AudioProcessor Constructor & Destructor
// ============================================================================
AudioProcessor::AudioProcessor(SPSCRingBuffer<float, 1024>& ringBuffer, AudioConfig config)
    : m_ringBuffer(ringBuffer), m_config(config), m_stream(nullptr) {}

AudioProcessor::~AudioProcessor() {
    (void)stopStream();
}

// ============================================================================
// 3. Audio Callback (Runs on Real-Time Audio Thread)
// ============================================================================
int AudioProcessor::recordCallback(
    const void* inputBuffer, 
    void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData
) {
    auto* ringBuffer = reinterpret_cast<SPSCRingBuffer<float, 1024>*>(userData);
    const float* in = reinterpret_cast<const float*>(inputBuffer);

    if (in && ringBuffer) {
        // Push the recorded samples into the lock-free queue
        for (unsigned long i = 0; i < framesPerBuffer; ++i) {
            ringBuffer->push(in[i]);
        }
    }
    
    return paContinue; // Instruct PortAudio to keep recording
}

// ============================================================================
// 4. Stream Control
// ============================================================================
std::expected<void, std::string> AudioProcessor::startStream() {
    PaStreamParameters inputParams;
    inputParams.device = Pa_GetDefaultInputDevice();
    if (inputParams.device == paNoDevice) {
        return std::unexpected("No default audio input device available.");
    }

    inputParams.channelCount = m_config.numInputChannels;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    PaStream* rawStream = nullptr;
    PaError err = Pa_OpenStream(
        &rawStream,
        &inputParams,
        nullptr, // Input-only stream, no output parameters
        m_config.sampleRate,
        m_config.framesPerBuffer,
        paClipOff, // Do not clip out-of-range samples
        &AudioProcessor::recordCallback,
        &m_ringBuffer // Pass the address of m_ringBuffer as userData
    );

    if (err != paNoError) {
        return std::unexpected(std::string("Failed to open PortAudio stream: ") + Pa_GetErrorText(err));
    }

    // Assign raw stream pointer to m_stream (auto-manages cleanup)
    m_stream.reset(rawStream);

    err = Pa_StartStream(m_stream.get());
    if (err != paNoError) {
        m_stream.reset(); // Safely stop and close the stream via custom deleter
        return std::unexpected(std::string("Failed to start PortAudio stream: ") + Pa_GetErrorText(err));
    }

    return {}; // Success
}

std::expected<void, std::string> AudioProcessor::stopStream() {
    if (m_stream) {
        m_stream.reset(); // Triggers PaStreamDeleter, stopping and closing the stream
    }
    return {};
}
