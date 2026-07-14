#include <iostream>
#include <thread>
#include <vector>
#include <complex>
#include <atomic>
#include <chrono>
#include <cmath>
#include <csignal>
#include <iomanip>
#include <algorithm>
#include "SPSCRingBuffer.h"
#include "AudioProcessor.h"
#include "FFTEngine.h"

constexpr size_t FFTSize = 1024;
constexpr size_t NumBands = 16;

// Global control flags
std::atomic<bool> g_running{true};
alignas(64) std::array<float, FFTSize / 2> g_spectrumMagnitudes{};
std::atomic<bool> g_newSpectrumAvailable{false};

// Signal handler to capture Ctrl+C and shut down cleanly
void signalHandler(int signal) {
    if (signal == SIGINT) {
        g_running.store(false, std::memory_order_relaxed);
    }
}

// DSP Consumer Thread Worker
void dspWorker(SPSCRingBuffer<float, 1024>& ringBuffer, FFTEngine& fftEngine) {
    std::vector<float> inputBuffer(FFTSize, 0.0f);
    std::vector<std::complex<float>> fftBuffer(FFTSize, std::complex<float>(0.0f, 0.0f));
    size_t sampleCount = 0;

    while (g_running.load(std::memory_order_relaxed)) {
        auto sample = ringBuffer.pop();
        if (sample) {
            inputBuffer[sampleCount++] = *sample;

            if (sampleCount == FFTSize) {
                // Apply Hamming window in-place on the time-domain floats
                fftEngine.applyWindow(inputBuffer);

                // Convert windowed floats to complex input
                for (size_t i = 0; i < FFTSize; ++i) {
                    fftBuffer[i] = std::complex<float>(inputBuffer[i], 0.0f);
                }

                // Compute forward FFT
                fftEngine.forward(fftBuffer);

                // Calculate magnitudes for first N/2 bins
                auto magnitudes = fftEngine.calculateMagnitudes(fftBuffer);

                // Atomically update shared spectrum data
                for (size_t i = 0; i < FFTSize / 2; ++i) {
                    g_spectrumMagnitudes[i] = magnitudes[i];
                }
                g_newSpectrumAvailable.store(true, std::memory_order_release);

                sampleCount = 0;
            }
        } else {
            // Queue empty, yield to prevent 100% CPU thread starvation
            std::this_thread::yield();
        }
    }
}

// Logarithmic Frequency Band Bin Ranges
struct FrequencyBand {
    std::string label;
    size_t startBin;
    size_t endBin;
};

// Generates logarithmic band ranges for 16 equalized bands at 44.1kHz sample rate
std::vector<FrequencyBand> generateFrequencyBands() {
    return {
        {"30-60 Hz   [Sub]  ", 1, 2},
        {"60-100 Hz  [Bass] ", 2, 3},
        {"100-150 Hz [Bass] ", 3, 4},
        {"150-250 Hz [Bass] ", 4, 6},
        {"250-400 Hz [Mids] ", 6, 10},
        {"400-600 Hz [Mids] ", 10, 15},
        {"600-1000 Hz[Mids] ", 15, 24},
        {"1-1.5 kHz  [Mids] ", 24, 35},
        {"1.5-2.5 kHz[Highs]", 35, 59},
        {"2.5-4 kHz  [Highs]", 59, 94},
        {"4-6 kHz    [Highs]", 94, 140},
        {"6-8 kHz    [Highs]", 140, 187},
        {"8-10 kHz   [Highs]", 187, 233},
        {"10-12 kHz  [Highs]", 233, 280},
        {"12-16 kHz  [Highs]", 280, 373},
        {"16-20 kHz  [Highs]", 373, 466}
    };
}

int main(int argc, char* argv[]) {
    // 1. Setup Signal Handler
    std::signal(SIGINT, signalHandler);

    std::cout << "Initializing Real-Time Audio DSP Pipeline..." << std::endl;

    // 2. Instantiate core components
    SPSCRingBuffer<float, 1024> ringBuffer;
    FFTEngine fftEngine(FFTSize);
    
    AudioConfig config;
    config.sampleRate = 44100.0;
    config.framesPerBuffer = 256; // Low latency block size
    config.numInputChannels = 1;

    AudioProcessor audioProcessor(ringBuffer, config);

    // 3. Start audio recording stream
    auto startResult = audioProcessor.startStream();
    if (!startResult) {
        std::cerr << "Initialization Error: " << startResult.error() << std::endl;
        return 1;
    }

    // 4. Spin up the background DSP thread
    std::thread dspThread(dspWorker, std::ref(ringBuffer), std::ref(fftEngine));

    // 5. Generate frequency band boundaries
    auto bands = generateFrequencyBands();
    
    // Clear screen initially
    std::cout << "\033[2J\033[H";

    // 6. Main thread UI loop (30 FPS)
    while (g_running.load(std::memory_order_relaxed)) {
        if (g_newSpectrumAvailable.exchange(false, std::memory_order_acquire)) {
            // Move cursor to top-left to redraw cleanly without flickering
            std::cout << "\033[H";
            std::cout << "========================================================================\n";
            std::cout << "         REAL-TIME AUDIO SPECTROGRAM VISUALIZER (C++23 & vcpkg)         \n";
            std::cout << "         Press Ctrl+C to terminate cleanly                              \n";
            std::cout << "========================================================================\n\n";

            for (const auto& band : bands) {
                // Calculate average magnitude in the band's bin range
                float sum = 0.0f;
                size_t count = 0;
                for (size_t bin = band.startBin; bin <= band.endBin && bin < (FFTSize / 2); ++bin) {
                    sum += g_spectrumMagnitudes[bin];
                    count++;
                }
                float avgMag = (count > 0) ? (sum / count) : 0.0f;

                // Convert magnitude to logarithmic decibel scale for visualization
                float db = 20.0f * std::log10(avgMag + 1e-5f);
                
                // Map dB scale [-60, 0] to bar width [0, 40]
                int barWidth = 0;
                if (db > -60.0f) {
                    barWidth = static_cast<int>((db + 60.0f) * 40.0f / 60.0f);
                }
                barWidth = std::clamp(barWidth, 0, 40);

                // Print the bar
                std::cout << band.label << " | ";
                for (int i = 0; i < barWidth; ++i) {
                    std::cout << "█";
                }
                // Clear remaining space to the right of the bar
                for (int i = barWidth; i < 40; ++i) {
                    std::cout << " ";
                }
                std::cout << " | " << std::fixed << std::setprecision(1) << db << " dB\n";
            }
            std::cout << "\n========================================================================\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(33)); // ~30 FPS
    }

    std::cout << "\nShutting down audio stream..." << std::endl;

    // 7. Clean up threads and streams
    // stopStream is automatically called by audioProcessor destructor, but calling it
    // explicitly ensures it stops before joining the worker thread.
    (void)audioProcessor.stopStream();
    if (dspThread.joinable()) {
        dspThread.join();
    }

    std::cout << "Pipeline terminated successfully." << std::endl;
    return 0;
}