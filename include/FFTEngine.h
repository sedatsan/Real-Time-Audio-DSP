#ifndef FFT_ENGINE_H
#define FFT_ENGINE_H

#include <vector>
#include <complex>
#include <span>

/**
 * @brief High-Performance C++ Fast Fourier Transform (FFT) Engine.
 * 
 * Design Features for Low Latency:
 * 1. In-place Cooley-Tukey Radix-2 algorithm (Decimation-in-Time).
 * 2. Pre-computed Twiddle Factors: Look-up table for e^(-i * 2 * pi * k / N) generated
 *    during construction to prevent calls to std::sin/std::cos in the hot DSP path.
 * 3. Pre-computed Windowing Weights: Hamming window coefficients calculated once.
 * 4. Zero-Copy Span Views: Uses std::span for zero-copy buffer passing.
 */
class FFTEngine {
public:
    // Size must be a power of 2 (e.g. 1024)
    explicit FFTEngine(size_t fftSize);
    ~FFTEngine() = default;

    // Non-copyable for safety
    FFTEngine(const FFTEngine&) = delete;
    FFTEngine& operator=(const FFTEngine&) = delete;

    /**
     * @brief Applies the Hamming window function in-place.
     * @param input Span of raw time-domain samples to window.
     */
    void applyWindow(std::span<float> input);

    /**
     * @brief Computes the in-place forward Radix-2 FFT.
     * @param data Input time-domain complex samples on entry.
     *             Output frequency-domain bins on exit.
     */
    void forward(std::span<std::complex<float>> data);

    /**
     * @brief Computes magnitude spectrum of the first half (up to Nyquist frequency).
     * @param fftOutput Full output of the forward FFT.
     * @return std::vector<float> Magnitudes for the first N/2 bins.
     */
    std::vector<float> calculateMagnitudes(std::span<const std::complex<float>> fftOutput);

private:
    /**
     * @brief Reorders complex data array by reversing the bits of their indices.
     *        This is the required precondition stage for the Cooley-Tukey butterfly.
     */
    void bitReversePermutation(std::span<std::complex<float>> data);
    
    /**
     * @brief Precomputes both the Hamming window weights and the Twiddle factors table.
     */
    void precomputeLookupTables();

    size_t m_fftSize;
    std::vector<float> m_windowCoefficients;
    std::vector<std::complex<float>> m_twiddleFactors;
};

#endif // FFT_ENGINE_H
