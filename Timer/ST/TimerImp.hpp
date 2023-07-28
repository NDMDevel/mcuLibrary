/*
 * SysTimer.h
 *
 *  Created on: Oct 4, 2021
 *      Author: Damian
 */

#ifndef SYSTIMER_HPP_
#define SYSTIMER_HPP_

#include "Timer/Timer.hpp"
#error "Replace this line with a proper #include \"xxxx.h\" that provide access to the SysTick funcionallity."
//#include "main.h"	//<- including main.h in a STCubeMX's project will provide access to SysTick functions

//System Timer
using SysTick = mcu::Timer<uint32_t,1,1000,HAL_GetTick>;

/*Other timers implementations, example Tim2:
using Tim32_us = mcu::Timer<uint32_t,1,1000000,
                            []{ return htim2.Instance->CNT; },
                            []{ HAL_TIM_Base_Start(&htim2); }>;

//Example of use:

void main()
{
    Tim32_us::hardwareInit();
    
    Tim32_us tim;
    //wait 100us
    while( tim < 100us );
    //do something...

    tim.restart();
    some_function();
    tim.stop();
    if( tim > 100ms )
        printf("some_function() took more than 100ms\n");
}
*/

#endif /* SYSTIMER_HPP_ */
