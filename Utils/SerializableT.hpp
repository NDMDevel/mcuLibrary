#pragma once

template<typename T>
union SerializableT
{
    T value;
    std::conditional_t<std::is_const_v<T>,const uint8_t,uint8_t> raw[sizeof(T)];
    SerializableT() = default;
    SerializableT(T src){ value = src; }
    SerializableT(const uint8_t src[sizeof(T)]){ copyFrom(src); }
    bool operator==(const uint8_t raw[sizeof(T)]) const
    {
        for( size_t idx=0 ; idx<sizeof(T) ; idx++ )
            if( raw[idx] != this->raw[idx] )
                return false;
        return true;
    }
    void copyFrom(const uint8_t src[sizeof(T)])
    {
        for( size_t idx=0 ; idx<sizeof(T) ; idx++ )
            raw[idx] = src[idx];
    }
    constexpr size_t size() const
    {
        return sizeof(T);
    }
};
