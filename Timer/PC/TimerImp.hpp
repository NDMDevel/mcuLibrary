#pragma once

#include "../Timer.hpp"

inline uint64_t sysTick_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
inline uint64_t sysTick_us()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
inline uint32_t sysTick32_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
inline uint32_t sysTick32_us()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

inline uint32_t tick_cnt = 0;
inline void tim_inc()
{
    tick_cnt++;
}
inline uint32_t tim_ms()
{
    return tick_cnt;
}

using Tim64_ms = mcu::Timer<uint64_t,1,1000     ,sysTick_ms>;
using Tim64_us = mcu::Timer<uint64_t,1,1000000  ,sysTick_us>;
using Tim32_ms = mcu::Timer<uint32_t,1,1000     ,sysTick32_ms>;
using Tim32_us = mcu::Timer<uint32_t,1,1000000  ,sysTick32_us>;

using Tim32_loop_us = mcu::Timer<uint32_t,1,1000000  ,tim_ms>;
using Tim32_loop_ms = mcu::Timer<uint32_t,1,1000     ,tim_ms>;
