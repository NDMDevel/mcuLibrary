#pragma once

#include <cstdint>
#include "../Utils/TypeUtils.hpp"

namespace mcu
{

template<typename T,size_t t_len>
class CircularSpan
{
public:
    using IdxType = fit_combinations_t<t_len>;
public:
    CircularSpan(const T* buff,IdxType len,IdxType head,IdxType tail)
        : _buff(buff), _len(len), _head(head) , _tail(tail){}
    IdxType size() const { return _len; }
    const T& operator[](IdxType idx) const
    {
        return itemAt(idx);
    }
private:
    const IdxType& itemAt(IdxType idx) const
    {
        if( _len == 0 )
            return _buff[_head];
        idx %= _len;
        if( _tail < _head )
            return _buff[_tail+idx];
        if( _tail+idx < _len )
            return _buff[_tail+idx];
        return _buff[_tail+idx-_len];
    }
private:
    const T* _buff;
    IdxType _len;
    IdxType _head;
    IdxType _tail;
};

}   //namespace mcu
