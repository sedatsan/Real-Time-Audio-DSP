#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include "FFTEngine.h"
#include "SPSCRingBuffer.h"

TEST_CASE("FFTEngine Benchmarks", "[benchmark][fft]") {
    FFTEngine engine(1024);
    std::vector<std::complex<float>> signal(1024, {1.0f, 0.0f});

    BENCHMARK("FFT 1024 Forward (Latency)") {
        engine.forward(signal);
        return signal[0];
    };
    
    BENCHMARK("FFT 1024 Window + Forward + Magnitudes") {
        std::vector<float> raw(1024, 0.5f);
        engine.applyWindow(raw);
        std::vector<std::complex<float>> c_signal(1024);
        for(size_t i=0; i<1024; ++i) c_signal[i] = raw[i];
        engine.forward(c_signal);
        return engine.calculateMagnitudes(c_signal);
    };
}

TEST_CASE("SPSCRingBuffer Benchmarks", "[benchmark][ringbuffer]") {
    SPSCRingBuffer<float, 4096> buffer;
    
    BENCHMARK("Push/Pop single item (Throughput / Latency)") {
        buffer.push(1.0f);
        return buffer.pop();
    };
}
