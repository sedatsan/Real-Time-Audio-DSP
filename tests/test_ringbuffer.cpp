#include <catch2/catch_test_macros.hpp>
#include "SPSCRingBuffer.h"

TEST_CASE("SPSCRingBuffer single-threaded behavior", "[ringbuffer]") {
    SPSCRingBuffer<int, 4> buffer;
    
    REQUIRE(buffer.empty());
    REQUIRE(buffer.size() == 0);
    
    REQUIRE(buffer.push(1) == true);
    REQUIRE(buffer.push(2) == true);
    REQUIRE(buffer.push(3) == true);
    REQUIRE(buffer.push(4) == false); // Capacity is N-1 = 3
    
    REQUIRE(!buffer.empty());
    REQUIRE(buffer.size() == 3);
    
    auto val1 = buffer.pop();
    REQUIRE(val1.has_value());
    REQUIRE(val1.value() == 1);
    
    buffer.pop();
    buffer.pop();
    
    REQUIRE(buffer.empty());
    REQUIRE(buffer.pop() == std::nullopt);
}
