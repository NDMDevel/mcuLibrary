/*
 * SysTimer.h
 *
 *  Created on: Oct 4, 2021
 *      Author: Damian
 */

#ifndef SYSTIMER_HPP_
#define SYSTIMER_HPP_

#include "Timer/Timer.hpp"
#error "Replaces this line with a proper #include "xxxx.h" that proved access to the SysTick funcionallity."
//#include "main.h"	//<- including main.h in a STCubeMX's project will provide access to SysTick functions

//System Timer
using SysTick = Timer<uint32_t,1,1000,HAL_GetTick>;

//Other timers implementations
//using Tim2_us = Timer<uint32_t,1,1000000,[]{ return *reinterpret_cast<uint32_t*>(TIM2_BASE + 0x24); }>;

#endif /* SYSTIMER_HPP_ */
