#pragma once

#include <array>
#include "../Utils/SerializableT.hpp"
#include "../Utils/TypeUtils.hpp"

namespace mcu
{

template<typename t_DataType,uint32_t t_len>
class LinearBuffer
{
private:
    using IdxType = fit_combinations_t<t_len>;
public:
    static constexpr IdxType t_maxLen = t_len;
public:
    void put(const t_DataType& data)
    {
        if( !isFull() )
            _buff[_head++] = data;
        else
            overflow = true;
    }
    const t_DataType* peek() const { return _buff.begin(); }
    t_DataType*       peek()  { return _buff.begin(); }
    IdxType length()    const { return _head; }
	IdxType freeSpace() const { return t_len-_head; }
    bool isFull()       const { return _head == t_len; }
    bool isEmpty()      const { return _head == 0; }
    bool isOverflowed() const { return overflow; }
    void clear(){ _head = 0; overflow = false; }
private:
	std::array<t_DataType,t_len> _buff;
	IdxType _head = 0;
    bool overflow = false;
};

}//namespace mcu
