#pragma once

#include "Timer/Timer.hpp"
#include <Arduino.h>

static uint32_t millis32()
{
    return millis();
}
static uint32_t micros32()
{
    return micros();
}
static uint64_t millis64()
{
    return millis();
}
static uint64_t micros64()
{
    return micros();
}


using Tim32_us = Timer<uint32_t,1,1000000,micros32>;
using Tim32_ms = Timer<uint32_t,1,   1000,millis32>;
using Tim64_us = Timer<uint64_t,1,1000000,micros64>;
using Tim64_ms = Timer<uint64_t,1,   1000,millis64>;
