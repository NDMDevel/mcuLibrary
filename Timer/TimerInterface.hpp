/*
 * TimerInterface.hpp
 *
 *  Created on: Apr 24, 2022
 *      Author: Damian
 */

#pragma once

#if defined(__cpp_lib_three_way_comparison)
    #include <compare>
#endif

#include <cstdint>
#include <chrono>
using namespace std::chrono_literals;

template<typename t_TimerResolution,intmax_t t_num,intmax_t t_den>
class TimerInterface
{
public:
    using IncPeriod = std::chrono::duration< t_TimerResolution , std::ratio<t_num,t_den> >;
    virtual void start() = 0;
    virtual void stop()  = 0;
    virtual bool running() const = 0;
    virtual IncPeriod elapsed() const = 0;
#if defined(__cpp_lib_three_way_comparison)
    virtual std::strong_ordering operator<=>(t_TimerResolution t) const = 0;
    virtual std::strong_ordering operator<=>(IncPeriod t) const = 0;
#else
    virtual bool operator< (t_TimerResolution t) const = 0;
    virtual bool operator<=(t_TimerResolution t) const = 0;
    virtual bool operator> (t_TimerResolution t) const = 0;
    virtual bool operator>=(t_TimerResolution t) const = 0;
    virtual bool operator==(t_TimerResolution t) const = 0;
    virtual bool operator!=(t_TimerResolution t) const = 0;

    virtual bool operator< (IncPeriod t) const = 0;
    virtual bool operator<=(IncPeriod t) const = 0;
    virtual bool operator> (IncPeriod t) const = 0;
    virtual bool operator>=(IncPeriod t) const = 0;
    virtual bool operator==(IncPeriod t) const = 0;
    virtual bool operator!=(IncPeriod t) const = 0;
#endif
};
