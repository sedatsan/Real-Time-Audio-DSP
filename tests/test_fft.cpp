#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "FFTEngine.h"

TEST_CASE("FFTEngine initialization and basic properties", "[fft]") {
    FFTEngine engine(1024);
    
    std::vector<std::complex<float>> signal(1024, {1.0f, 0.0f});
    engine.forward(signal);
    
    auto mags = engine.calculateMagnitudes(signal);
    REQUIRE(mags.size() == 512);
    REQUIRE(mags[0] > 10.0f); // High DC component
    for (size_t i = 1; i < 512; ++i) {
        REQUIRE_THAT(mags[i], Catch::Matchers::WithinAbs(0.0f, 0.001f));
    }
}
