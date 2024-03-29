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
    CircularSpan()
        : _buff(nullptr), _len(0), _tail(0), _head(0){}
    CircularSpan(const T* buff,IdxType len,IdxType tail,IdxType head)
        : _buff(buff), _len(len), _tail(tail), _head(head){}
    IdxType size()   const { return _len; }
    IdxType length() const { return _len; }
    const T& operator[](IdxType idx) const
    {
        return itemAt(idx);
    }
private:
    const T& itemAt(IdxType idx) const
    {
        if( size() == 0 )
            return _buff[_head];
        idx %= size();
        if( _tail < _head )
            return _buff[_tail+idx];
        if( _tail+idx < t_len )
            return _buff[_tail+idx];
        return _buff[_tail+idx-t_len];
    }
private:
    const T* _buff;
    IdxType _len;
    IdxType _tail;
    IdxType _head;
};

}   //namespace mcu
