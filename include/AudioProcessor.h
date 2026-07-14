#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSOR_H

#include <expected>
#include <string>
#include <memory>
#include <span>
#include "portaudio.h"
#include "SPSCRingBuffer.h"

// Structured configuration
struct AudioConfig {
    double sampleRate = 44100.0;
    unsigned long framesPerBuffer = 1024;
    int numInputChannels = 1; // Mono recording
};

// Global PortAudio Library Context RAII Manager
class PortAudioContext {
public:
    PortAudioContext();
    ~PortAudioContext();
    
    // Non-copyable/Non-movable
    PortAudioContext(const PortAudioContext&) = delete;
    PortAudioContext& operator=(const PortAudioContext&) = delete;
};

// Main Audio Processor class
class AudioProcessor {
public:
    // Pass a reference to the lock-free queue that the audio thread will write to
    explicit AudioProcessor(SPSCRingBuffer<float, 1024>& ringBuffer, AudioConfig config = {});
    ~AudioProcessor();

    // Prevent copies
    AudioProcessor(const AudioProcessor&) = delete;
    AudioProcessor& operator=(const AudioProcessor&) = delete;

    // Initialize stream. Returns expected void on success, string error on failure.
    std::expected<void, std::string> startStream();
    std::expected<void, std::string> stopStream();

private:
    // C++ unique_ptr wrapper for C-style PaStream resource using a custom deleter
    struct PaStreamDeleter {
        void operator()(PaStream* stream) const {
            if (stream) {
                // If stream is active, stop it before closing
                if (Pa_IsStreamActive(stream) == 1) {
                    Pa_StopStream(stream);
                }
                Pa_CloseStream(stream);
            }
        }
    };
    using UniquePaStream = std::unique_ptr<PaStream, PaStreamDeleter>;

    // The static callback matching PortAudio's C-style signature
    static int recordCallback(
        const void* inputBuffer, 
        void* outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData
    );

    SPSCRingBuffer<float, 1024>& m_ringBuffer;
    AudioConfig m_config;
    UniquePaStream m_stream;
    PortAudioContext m_paContext;
};

#endif // AUDIO_PROCESSOR_H
