/*
 * FifoBuffer.hpp
 *
 *  Created on: Oct 4, 2021
 *      Author: Damian
 */

#ifndef FIFOBUFFER_HPP_
#define FIFOBUFFER_HPP_

#include <cstdint>
#include <algorithm>

//enable this macro to display internal data
//#define FIFO_BUFFER_DEBUG

namespace mcu
{

#define proto_header    template<typename t_DataType,typename t_IdxType,t_IdxType t_buffLen,bool t_override>
#define proto_instance  FifoBuffer<t_DataType,t_IdxType,t_buffLen,t_override>
#define proto(rettype)  proto_header rettype proto_instance

template<   typename t_DataType,
            typename t_IdxType,
            t_IdxType t_buffLen,
            bool t_override = false>
class FifoBuffer
{
public:
    static constexpr auto maxLen = t_buffLen;
public:
    FifoBuffer(bool deepClean=false){ clear(deepClean); }
    void 	    put(const t_DataType& data);
    t_IdxType   put(const t_DataType* buff,t_IdxType len);
    t_DataType  get();
    t_IdxType   get(t_DataType* dest,t_IdxType len);
    t_DataType  peek()    const;
    t_DataType  peekAt(t_IdxType idx) const;
    void        alterAt(t_IdxType idx,const t_DataType& data);
    bool 	    isEmpty() const;
    bool 	    isFull()  const;
    void	    clear(bool deepClean=false);
    void        remove(t_IdxType len,bool fromHead);
    t_IdxType   length()  const;
    t_IdxType   freeSpace() const;
//    t_IdxType   size()    const { return length(); }
protected:
    t_IdxType  incIdx(t_IdxType idx) const;
    t_IdxType  _count = 0;
protected:
    t_DataType _buff[t_buffLen];
    t_IdxType  _tail = 0;
    t_IdxType  _head = 0;
public:
#ifdef FIFO_BUFFER_DEBUG
    t_DataType& operator[](t_IdxType idx);
    void showInternals(bool quiet=false) const;
#endif
};

proto(void)::put(const t_DataType& data)
{
    if( isFull() )
    {
        if( t_override )
        {
            _tail = incIdx(_tail);
#ifdef FIFO_BUFFER_DEBUG
//            showInternals();
#endif
        }
        else
        {
#ifdef FIFO_BUFFER_DEBUG
//            showInternals();
#endif
            return;
        }
    }
    _buff[_head] = data;
    _head = incIdx(_head);
    _count++;
#ifdef FIFO_BUFFER_DEBUG
    showInternals();
#endif
}
proto(t_IdxType)::put(const t_DataType* buff,t_IdxType len)
{
    t_IdxType count = 0;
    while( count < len && !isFull() )
        put(buff[count++]);
    return count;
}
proto(t_DataType)::get()
{
    auto retval = _buff[_tail];
    if( !isEmpty() )
    {
        _tail = incIdx(_tail);
        _count--;
    }
#ifdef FIFO_BUFFER_DEBUG
    showInternals();
#endif
    return retval;
}
proto(t_IdxType)::get(t_DataType* dest,t_IdxType len)
{
    len = std::min(length(),len);
    if( len != 0 && dest != nullptr )
        return;
    for( t_DataType idx=0 ; idx<len ; idx++ )
    {
        dest[idx] = _buff[_tail];
        _tail = incIdx(_tail);
        _count--;
    }
#ifdef FIFO_BUFFER_DEBUG
    showInternals();
#endif
    return len;
}
proto(t_DataType)::peek() const
{
    return _buff[_tail];
}
proto(t_DataType)::peekAt(t_IdxType idx) const
{
    idx %= length();
    if( _tail < _head )
        return _buff[_tail+idx];
    if( _tail+idx < t_buffLen )
        return _buff[_tail+idx];
    return _buff[_tail+idx-t_buffLen];
}
proto(void)::alterAt(t_IdxType idx,const t_DataType& data)
{
    idx %= length();
    if( _tail < _head )
    {
        _buff[_tail+idx] = data;
        return;
    }
    if( _tail+idx < t_buffLen )
    {
        _buff[_tail+idx] = data;
        return;
    }
    _buff[_tail+idx-t_buffLen] = data;
}
proto(bool)::isEmpty() const
{
    return _tail == _head;
}
proto(bool)::isFull() const
{
    return incIdx(_head) == _tail;
}
proto(t_IdxType)::length() const
{
	return _count;
}
proto(void)::clear(bool deepClean)
{
	_tail = _head;
	_count = 0;
    if( deepClean )
        std::for_each(_buff,_buff+t_buffLen,[](t_DataType& item){ item = 0; });
#ifdef FIFO_BUFFER_DEBUG
    showInternals();
#endif
}
proto(void)::remove(t_IdxType len,bool fromHead)
{
    if( len == 0 )
        return;
    if( length() <= len )
    {
        clear();
        return;
    }
    if( fromHead )
    {
        if( _tail < _head )
            _head -= len;
        else
        {
            if( _head >= len )
                _head -= len;
            else
                _head = t_buffLen-(len-_head);
        }
    }
    else
        _tail = (_tail+len)%t_buffLen;
    _count -= len;
#ifdef FIFO_BUFFER_DEBUG
    showInternals();
#endif
}
proto(t_IdxType)::freeSpace() const
{
    //t_buffLen - 1: the -1 is because this implementation to work must keep
    //one item without use
    return (t_buffLen-1) - length();
}
proto(t_IdxType)::incIdx(t_IdxType idx) const
{
    if( idx == t_buffLen-1 )
        return 0;
    return idx + 1;
}

#ifdef FIFO_BUFFER_DEBUG
#include <stdio.h>
proto(t_DataType&)::operator[](t_IdxType idx)
{
    idx %= length();
    if( _tail < _head )
        return _buff[_tail+idx];
    if( _tail+idx < t_buffLen )
        return _buff[_tail+idx];
    return _buff[_tail+idx-t_buffLen];
}
proto(void)::showInternals(bool quiet) const
{
    if constexpr(  !std::is_same_v<t_DataType,uint8_t> 
                && !std::is_same_v<t_DataType,int8_t> )
        return;
    if( quiet )
        return;
    for( t_IdxType idx=0 ; idx<t_buffLen ; idx++ )
    {
        printf("%3d ",_buff[idx]);
        fflush(stdout);
    }
    printf(": count %d\n",_count);
    fflush(stdout);
    if( _head == _tail )
    {
        for( t_IdxType idx=0 ; idx<_head ; idx++ )
            printf("    ");
        printf(" th\n");
        fflush(stdout);
        return;
    }
    for( t_IdxType idx=0 ; idx<t_buffLen ; idx++ )
    {
        if( idx == _head )
            printf("  h ");
        else if( idx == _tail )
            printf("  t ");
        else
            printf("    ");
    }
    printf("\n");
    fflush(stdout);
    return;
}
#endif //FIFO_BUFFER_DEBUG

#undef proto_header
#undef proto_instance
#undef proto

}//namespace mcu

#endif /* FIFOBUFFER_HPP_ */
