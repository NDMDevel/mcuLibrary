#pragma once

#include <cstdint>
#include <type_traits>

template<uint64_t N,typename T>
consteval bool fits_on(){ return N <= T(~uint64_t(0)); }

template<uint64_t N>
using fit_type_t =
    std::conditional_t<fits_on<N,uint8_t>() ,uint8_t,
    std::conditional_t<fits_on<N,uint16_t>(),uint16_t,
    std::conditional_t<fits_on<N,uint32_t>(),uint32_t,uint64_t>>>;


//credits to https://stackoverflow.com/a/28796458/2538072
template<typename Test, template<typename...> class Ref>
struct is_specialization : std::false_type {};

template<template<typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref>: std::true_type {};
