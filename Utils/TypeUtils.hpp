#pragma once

#include <cstdint>
#include <type_traits>

namespace mcu
{

/**
 * fits_on<N,T>()
 * 
 * Checks if the type T can store the value N without overflow.
 * Examples:
 *      fits_on< 100,uint8_t>() will return true
 *      fits_on<1000,uint8_t>() will return false
 *      fits_on< 255,uint8_t>() will return true
 *      fits_on< 256,uint8_t>() will return false
 * 
 * 
 * fit_value_t<N>
 * 
 * Alias a type that can store N without overflow.
 * Examples:
 *      fit_value_t<255> x; //x is of type uint8_t
 *      fit_value_t<256> x; //x is of type uint16_t
 * 
 * 
 * fit_combinations_t<N>
 * 
 * Alias a type that can store N combinations without overflow.
 * Examples:
 *      fit_count_t<255> x; //x is of type uint8_t
 *      fit_count_t<256> x; //x is of type uint8_t
 *      fit_value_t<257> x; //x is of type uint16_t
*/
#if __cplusplus >= 202002L
    template<uint64_t N,typename T>
    consteval bool fits_on(){ return N <= T(~uint64_t(0)); }

    template<uint64_t N>
    using fit_combinations_t =
        std::conditional_t< N == 0                ,void,
        std::conditional_t<fits_on<N-1,uint8_t>() ,uint8_t,
        std::conditional_t<fits_on<N-1,uint16_t>(),uint16_t,
        std::conditional_t<fits_on<N-1,uint32_t>(),uint32_t,uint64_t>>>>;
    template<uint64_t N>
    using fit_value_t =
        std::conditional_t<fits_on<N,uint8_t>() ,uint8_t,
        std::conditional_t<fits_on<N,uint16_t>(),uint16_t,
        std::conditional_t<fits_on<N,uint32_t>(),uint32_t,uint64_t>>>;

    template<typename T>
    consteval fit_value_t<static_cast<T>(~size_t(0))> max_val_storable_on(){ return static_cast<T>(~size_t(0)); }

#else
    template<uint64_t N>
    using fit_combinations_t =
        std::conditional_t<N<=uint8_t (~uint64_t(0))+1,uint8_t,
        std::conditional_t<N<=uint16_t(~uint64_t(0))+1,uint16_t,
        std::conditional_t<N<=uint32_t(~uint64_t(0))+1,uint32_t,uint64_t>>>;
    template<uint64_t N>
    using fit_value_t =
        std::conditional_t<N<=uint8_t (~uint64_t(0)),uint8_t,
        std::conditional_t<N<=uint16_t(~uint64_t(0)),uint16_t,
        std::conditional_t<N<=uint32_t(~uint64_t(0)),uint32_t,uint64_t>>>;

    template<typename T>
    consteval T max_val_storable_on(){ return static_cast<fit_value_t<T>>(~size_t(0)); }
#endif

//credits to https://stackoverflow.com/a/28796458/2538072
template<typename Test, template<typename...> class Ref>
struct is_specialization : std::false_type {};

template<template<typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref>: std::true_type {};

}//namespace mcu
