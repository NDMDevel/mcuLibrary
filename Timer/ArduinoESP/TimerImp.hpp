#pragma once

#include "Timer/Timer.hpp"

static inline uint32_t millis32()
{
    return millis();
}
static inline uint32_t micros32()
{
    return micros();
}
static inline uint64_t millis64()
{
    return millis();
}
static inline uint64_t micros64()
{
    return micros();
}

using Tim32_us = mcu::Timer<uint32_t,1,1000000,micros32>;
using Tim32_ms = mcu::Timer<uint32_t,1,   1000,millis32>;
using Tim64_us = mcu::Timer<uint64_t,1,1000000,micros64>;
using Tim64_ms = mcu::Timer<uint64_t,1,   1000,millis64>;
