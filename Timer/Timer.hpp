/*
 * TimerInterface.hpp
 *
 *  Created on: Apr 24, 2022
 *      Author: Damian
 */

#pragma once

#include <chrono>
#include <cstdint>

using namespace std::chrono_literals;

namespace mcu
{

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
template<   typename t_TimerResolution,
            intmax_t t_Num,
            intmax_t t_Den,
            t_TimerResolution(*t_GetTime)(),
            void(*t_HardwareInit)() = nullptr>
class Timer
{
private:
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
public: //exported types
    using TimerResolution = t_TimerResolution;
    static constexpr auto Num = t_Num;
    static constexpr auto Den = t_Den;
    using IncPeriod = std::chrono::duration< t_TimerResolution , std::ratio<t_Num,t_Den> >;
public:
    //seconds to overflow (timer module)
    static constexpr auto overflow_time = t_TimerResolution(ldouble_t(t_TimerResolution(-1))*ldouble_t(t_Num)/ldouble_t(t_Den));
    static constexpr void hardwareInit() { t_HardwareInit(); }
public:
    Timer(bool start=true) : _tick(0),_running(start){ if(start) this->start(); }
    void restart(){ start(); }
    void start() 
    {
        _tick = ~t_GetTime() + t_TimerResolution(1);
        _running = true;
    }
    void stop() 
    {
        _tick = elapsed().count();
        _running = false;
    }
    bool running() const 
    {
    	return _running;
    }
    IncPeriod elapsed() const 
    {
        if( _running )
            return IncPeriod(_tick + t_TimerResolution(t_GetTime()));
        return IncPeriod(_tick);
    }
#if defined(__cpp_lib_three_way_comparison)
    std::strong_ordering operator<=>(t_TimerResolution t) const 
    {
        return elapsed().count() <=> t;
    }
    std::strong_ordering operator<=>(IncPeriod t) const 
    {
        return elapsed().count() <=> t.count();
    }
#else
    bool operator< (t_TimerResolution t) const 
    {
        return elapsed().count() < t;
    }
    bool operator<=(t_TimerResolution t) const 
    {
        return elapsed().count() <= t;
    }
    bool operator> (t_TimerResolution t) const 
    {
        return elapsed().count() > t;
    }
    bool operator>=(t_TimerResolution t) const 
    {
        return elapsed().count() >= t;
    }
    bool operator==(t_TimerResolution t) const
    {
        return elapsed().count() == t;
    }
    bool operator!=(t_TimerResolution t) const 
    {
        return elapsed().count() != t;
    }

    bool operator< (IncPeriod t) const 
    {
        return elapsed().count() < t.count();
    }
    bool operator<=(IncPeriod t) const 
    {
        return elapsed().count() <= t.count();
    }
    bool operator> (IncPeriod t) const 
    {
        return elapsed().count() > t.count();
    }
    bool operator>=(IncPeriod t) const 
    {
        return elapsed().count() >= t.count();
    }
    bool operator==(IncPeriod t) const
    {
        return elapsed().count() == t.count();
    }
    bool operator!=(IncPeriod t) const 
    {
        return elapsed().count() != t.count();
    }
#endif
    void operator-=(t_TimerResolution dt)
    {
        _tick -= dt;
    }
    void operator-=(IncPeriod dt)
    {
        _tick -= dt.count();
    }
    TimerResolution getTick() const
    {
        return _tick;
    }
private:
    t_TimerResolution _tick;
    bool _running;
};

}//namespace mcu

