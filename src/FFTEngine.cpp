#include "FFTEngine.h"
#include <cmath>
#include <complex>
#include <cstddef>
#include <iostream>
#include <bit>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

FFTEngine::FFTEngine(size_t fftSize) : m_fftSize(fftSize) {
    m_windowCoefficients.resize(m_fftSize);
    m_twiddleFactors.resize(m_fftSize);
    precomputeLookupTables();
}

void FFTEngine::precomputeLookupTables() {
    // 1. Precompute Hamming window coefficients
    for (size_t i = 0; i < m_fftSize; i++) {
        m_windowCoefficients[i] = 0.54f - 0.46f * std::cos(2.0f * static_cast<float>(M_PI) * i / (m_fftSize - 1));
    }

    // 2. Precompute Twiddle Factors (Roots of Unity) for FFT
    for (size_t k = 0; k < m_fftSize; k++) {
        float theta = static_cast<float>(-2.0 * M_PI * k / m_fftSize);
        m_twiddleFactors[k] = std::complex<float>(std::cos(theta), std::sin(theta));
    }
}

void FFTEngine::applyWindow(std::span<float> input) {
    if (input.size() != m_fftSize) {
        return;
    }
    for (size_t i = 0; i < m_fftSize; i++) {
        input[i] *= m_windowCoefficients[i];
    }
}

void FFTEngine::bitReversePermutation(std::span<std::complex<float>> data) {
    size_t n = data.size();
    size_t log2_n = std::countr_zero(n);

    for (size_t i = 0; i < n; i++) {
        size_t j = 0;
        size_t temp = i;
        for (size_t bit = 0; bit < log2_n; bit++) {
            j = (j << 1) | (temp & 1);
            temp >>= 1;
        }
        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }
}

void FFTEngine::forward(std::span<std::complex<float>> data) {
    size_t n = data.size();
    if (n != m_fftSize) return;

    // 1. Perform bit-reversal reordering stage
    bitReversePermutation(data);

    // 2. Perform Cooley-Tukey butterfly merge stages
    for (size_t len = 2; len <= n; len <<= 1) {
        size_t halfLen = len / 2;
        for (size_t step = 0; step < n; step += len) {
            for (size_t k = 0; k < halfLen; ++k) {
                size_t twiddle_idx = k * (n / len);
                std::complex<float> t = data[step + k + halfLen] * m_twiddleFactors[twiddle_idx];
                std::complex<float> u = data[step + k];
                data[step + k] = u + t;
                data[step + k + halfLen] = u - t;
            }
        }
    }
}

std::vector<float> FFTEngine::calculateMagnitudes(std::span<const std::complex<float>> fftOutput) {
    std::vector<float> magnitudes;
    magnitudes.reserve(m_fftSize / 2);

    for (size_t i = 0; i < m_fftSize / 2; ++i) {
        magnitudes.push_back(std::abs(fftOutput[i]));
    }

    return magnitudes;
}
