#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include "../Utils/SerializableT.hpp"
#include "../Utils/TypeUtils.hpp"
#include "CircularSpan.hpp"

#ifdef DEBUG_VLITEMFIFO
#include <iostream>
#endif

namespace mcu
{

template<typename    t_DataType,
         size_t      t_buffLen,
         size_t      t_itemLen = std::min<size_t>(t_buffLen,
                                                  max_val_storable_on<fit_value_t<t_buffLen>>())//static_cast<fit_value_t<t_buffLen>>(~size_t(0))
        >
class VLItemFifo
{
    static_assert( t_buffLen > t_itemLen , "t_buffLen must be greater than t_itemLen" );
public:
    static constexpr auto maxLen = t_buffLen;
    using IdxType = fit_combinations_t<t_buffLen>;
private:
    using SofType = fit_value_t<t_itemLen>;
public:
    VLItemFifo(){ clear(); }
    auto commitItem() -> void
    {
        if( _emptyItem )
            return;
        _sof = _head;
        _itemsCount++;
        _emptyItem = true;
    }
    /*auto append(uint8_t data) -> void
    {
        if( isFull() )
            return;
        _buff[_head] = data;
        _head = incIdx(_head);
        _emptyItem = false;
    }*/
    template<typename T>
    auto push(T data) -> void
    {
//        static_assert( sizeof(T) <  t_buffLen , "cant push data bigger than the container size (VLItemFifo<T,N>::maxLen)" );
        static_assert( sizeof(T) <= t_itemLen , "cant push data bigger than the maximun data lentgh (t_itemLen)" );
        if( size_t(currentItemLength()) + sizeof(T) > t_itemLen )
        {
            std::cout << "push ignored to avoid overflow\n";
            return;
        }
        size_t neededSpace = size_t(rawLength()) + sizeof(T);
        if( _emptyItem )
            neededSpace += sizeof(SofType);
        if( neededSpace > t_buffLen )
        {
            std::cout << "push ignored to avoid overflow\n";
            return;
        }
        if( _emptyItem )
        {
            _emptyItem = false;
            clrSof();
            for( SofType i=0 ; i<sizeof(SofType) ; i++ )
                incHead();
        }
        SerializableT<T> sdata;
        sdata.value = data;
        for( const auto& item : sdata.raw )
        {
            _buff[_head] = item;
            incHead();
        }
        incSof(sizeof(T));
    }
    auto pop() -> void //std::optional<T>
    {
        if( isEmpty() )
            return;
        _tail = incIdx(_tail,firstItemLength()+sizeof(SofType));
        _itemsCount--;
    }
    auto peek() const -> std::optional<t_DataType>
    {
    	if( itemsCount() == 0 )
    		return std::nullopt;
        IdxType tail = _tail;
        for( SofType i=0 ; i<sizeof(SofType) ; i++ )
            tail = incIdx(tail);
    	return _buff[tail];
    }
    CircularSpan<t_DataType,t_buffLen> getCircularSpan() const
    {
        IdxType tail = _tail;
        for( SofType i=0 ; i<sizeof(SofType) ; i++ )
            tail = incIdx(tail);
        std::cout << "tail: " << int(tail) << " firstItemLength(): " << int(firstItemLength()) << std::endl;
        return CircularSpan<t_DataType,t_buffLen>(_buff.data(),firstItemLength(),tail,_head);
    }
    auto clear() -> void
    {
        _head = 0;
        _tail = 0;
        _itemsCount = 0;
        _sof  = 0;
        clrSof();
        _emptyItem = true;
    }
    auto clearLastItem() -> void
    {
        if( _itemsCount == 0 )
            return;
        _itemsCount--;
        _head =_sof;
        clrSof();
        _emptyItem = true;
    }
    auto currentItemLength() const -> SofType
    {
        if( _emptyItem )
            return 0;
        return getSof();
    }
    auto firstItemLength() const -> SofType
    {
        if( _itemsCount == 0 )
            return 0;
        SerializableT<SofType> ssof;
        IdxType sof = _tail;
        for( auto& s : ssof.raw )
        {
            s = _buff[sof];
            sof = incIdx(sof);
        }
        return ssof.value;
    }
    auto isEmpty() const -> bool { return _tail == _head; }
    auto isFull()  const -> bool { return incIdx(_head) == _tail; }
    auto itemsCount() const -> IdxType { return _itemsCount; }
#ifdef DEBUG_VLITEMFIFO
    auto print_internals() const -> void
    {
        std::cout << "_sof[" << sizeof(SofType) << " Bytes]: " << int(_sof) << std::endl;
        for( size_t i=0 ; i<_buff.size() ; i++ )
        {
            if( i == _sof )
                std::cout << "*";
            if( i == _head )
                std::cout << "h[" << int(_buff[i]) << "] ";
            else if( i == _tail )
                std::cout << "t[" << int(_buff[i]) << "] ";
            else
                std::cout << int(_buff[i]) << " ";
        }
        std::cout << std::endl;
    }
#endif
protected:
    auto incIdx(IdxType idx) const -> IdxType
    {
        if( idx == t_buffLen-1 )
            return 0;
        return idx + 1;
    }
    auto incHead() -> void{ _head = incIdx(_head); }
    auto incTail() -> void{ _tail = incIdx(_tail); }
    auto rawLength()  const -> IdxType
    {
        if( isFull() )
            return t_buffLen - 1;
        if( _tail <= _head )
            return _head - _tail;
        return t_buffLen - _tail + _head;
    }
    auto clrSof() -> void
    {
        SerializableT<SofType> ssof;
        ssof.value = 0;
        IdxType isof = _sof;
        for( const auto& s : ssof.raw )
        {
            _buff[isof] = s;
            isof = incIdx(isof);
        }
    }
    auto getSof() const -> SofType
    {
        SerializableT<SofType> ssof;
        IdxType isof = _sof;
        for( auto& s : ssof.raw )
        {
            s = _buff[isof];
            isof = incIdx(isof);
        }
        return ssof.value;
    }
    auto incSof(SofType len) -> void
    {
        SerializableT<SofType> ssof;
        IdxType isof = _sof;
        for( auto& s : ssof.raw )
        {
            s = _buff[isof];
            isof = incIdx(isof);
        }
        ssof.value += len;
        isof = _sof;
        for( const auto& s : ssof.raw )
        {
            _buff[isof] = s;
            isof = incIdx(isof);
        }
    }
    auto incIdx(IdxType idx,IdxType len) const -> IdxType { return (idx+len)%t_buffLen; }
protected:
    std::array<t_DataType,t_buffLen> _buff;
    IdxType     _tail   = 0;
    IdxType     _head   = 0;
    IdxType     _itemsCount = 0;
    IdxType     _sof    = 0;
    bool        _emptyItem = true;
};

}//namespace mcu
