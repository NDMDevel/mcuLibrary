#pragma once

#include "Timer/Timer.hpp"

inline uint64_t sysTick()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

using Timer = mcu::Timer<uint64_t,1,1000,sysTick>;
