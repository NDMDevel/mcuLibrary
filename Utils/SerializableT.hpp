#pragma once
#include <cstdint>

template<uint64_t N,typename T>
consteval bool fits_on(){ return N <= T(~uint64_t(0)); }

template<uint64_t N>
using fit_type_t =
    std::conditional_t<fits_on<N,uint8_t>(),uint8_t,
    std::conditional_t<fits_on<N,uint16_t>(),uint16_t,
    std::conditional_t<fits_on<N,uint32_t>(),uint32_t,uint64_t>>>;


template<typename T>
union SerializableT
{
    T value;
    std::conditional_t<std::is_const_v<T>,const uint8_t,uint8_t> raw[sizeof(T)];
    bool operator==(const uint8_t raw[sizeof(T)])
    {
        for( uint32_t idx=0 ; idx<sizeof(T) ; idx++ )
            if( raw[idx] != this->raw[idx] )
                return false;
        return true;
    }
};
