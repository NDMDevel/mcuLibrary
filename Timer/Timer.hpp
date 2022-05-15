/*
 * TimerInterface.hpp
 *
 *  Created on: Apr 24, 2022
 *      Author: Damian
 */

#ifndef PERIPH_TIMER_HPP_
#define PERIPH_TIMER_HPP_

#include "TimerInterface.hpp"
#include <chrono>
#include <cstdint>

/** @brief Timer class
 *
 * Template @arg TimerResolution define the bit resolution
 * of the timer and the valid values are:
 *  uint8_t
 *  uint16_t
 *  uint32_t
 *  uint64_t
 *
 * Template @arg t_Num and @arg t_Den defines the delta-time
 * in seconds between increments (increment's period)
 * as a ratio:
 *  IncPeriod = t_Num/t_Den.
 * For instance to define a timer that increments each 37.5us
 * t_Num = 3, and t_Den = 80000.
 *
 * Template @arg t_GetTime is a function pointer used by the Timer
 * class to read the current state of the timer.
 *
 * ARDUINO note:
 * For arduino boards, this function may be micros() or millis(),
 * but the return type of @arg t_GetTime must be @arg t_TimerResolution
 * and if not the same as micros()/millis() a compiler error will arise.
 * To avoid this, a function wrapper may be created around micros()/millis().
 * For instance, if @arg t_TimerResolution is uint32_t, and assuming
 * that the prototype of micros() is 'uint64_t micros(void)', the wrapper
 * should be implemented as:
 *  uint32_t micros_wraper()
 *  {
 *   uint32_t us = micros();
 *   return us;
 *  }
 */
template<typename t_TimerResolution,intmax_t t_Num,intmax_t t_Den,t_TimerResolution(*t_GetTime)(void)>
class Timer : public TimerInterface<t_TimerResolution,t_Num,t_Den>
{
    static_assert(
        std::is_same<t_TimerResolution,std::uint8_t> ::value ||
        std::is_same<t_TimerResolution,std::uint16_t>::value ||
        std::is_same<t_TimerResolution,std::uint32_t>::value ||
        std::is_same<t_TimerResolution,std::uint64_t>::value ,
        "t_TimerResolution must be one of: "
        "uint8_t, uint16_t, uint32_t or uint64_t");
    static_assert(t_Num != 0,"t_Num must be greater than zero");
    static_assert(t_Den != 0,"t_Den must be greater than zero");
    static_assert(t_GetTime != nullptr,"t_GetTime must be not nullptr");
    using ldouble_t = long double;
public:
    using IncPeriod = std::chrono::duration< t_TimerResolution , std::ratio<t_Num,t_Den> >;
public:
    //seconds to overflow (timer module)
    static constexpr auto overflow_time = uint32_t(ldouble_t(t_TimerResolution(-1))*ldouble_t(t_Num)/ldouble_t(t_Den));
public:
    Timer() : _tick(0),_running(false){}
    void restart(){ start(); }
    //interface methods
    void start() override
    {
        _tick = ~t_GetTime() + t_TimerResolution(1);
        _running = true;
    }
    void stop() override
    {
        _tick = elapsed().count();
        _running = false;
    }
    bool running() const override
    {
    	return _running;
    }
    IncPeriod elapsed() const override
    {
        if( _running )
            return IncPeriod(_tick + t_TimerResolution(t_GetTime()));
        return IncPeriod(_tick);
    }
#if defined(TIMER_SUPPORT_THREE_WAY_CMP)
    std::strong_ordering operator<=>(t_TimerResolution t) const override
    {
        return elapsed().count() <=> t;
    }
    std::strong_ordering operator<=>(IncPeriod t) const override
    {
        return elapsed().count() <=> t.count();
    }
#else
    bool operator< (t_TimerResolution t) const override
    {
        return elapsed().count() < t;
    }
    bool operator<=(t_TimerResolution t) const override
    {
        return elapsed().count() <= t;
    }
    bool operator> (t_TimerResolution t) const override
    {
        return elapsed().count() > t;
    }
    bool operator>=(t_TimerResolution t) const override
    {
        return elapsed().count() >= t;
    }
    bool operator==(t_TimerResolution t) const override
    {
        return elapsed().count() == t;
    }
    bool operator!=(t_TimerResolution t) const override
    {
        return elapsed().count() != t;
    }

    bool operator< (IncPeriod t) const override
    {
        return elapsed().count() < t.count();
    }
    bool operator<=(IncPeriod t) const override
    {
        return elapsed().count() <= t.count();
    }
    bool operator> (IncPeriod t) const override
    {
        return elapsed().count() > t.count();
    }
    bool operator>=(IncPeriod t) const override
    {
        return elapsed().count() >= t.count();
    }
    bool operator==(IncPeriod t) const override
    {
        return elapsed().count() == t.count();
    }
    bool operator!=(IncPeriod t) const override
    {
        return elapsed().count() != t.count();
    }
#endif
private:
    t_TimerResolution _tick;
    bool _running;
};

#endif /* PERIPH_TIMER_HPP_ */
