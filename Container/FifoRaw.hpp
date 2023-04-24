#pragma once

#include <algorithm>

namespace mcu
{

template<   typename t_DataType,
            typename t_IdxType,
            t_IdxType t_buffLen,
            bool t_override = false>
class FifoRaw
{
public:
    static constexpr auto maxLen = t_buffLen;
public:
    FifoRaw(bool deepClean=false){ clear(deepClean); }
    void 	    put(const t_DataType& data)
    {
        if( isFull() )
        {
            if( t_override )
                _tail = incIdx(_tail);
            else
                return;
        }
        _buff[_head] = data;
        _head = incIdx(_head);
        _count++;
    }
    t_IdxType   put(const t_DataType* buff,t_IdxType len)
    {
        t_IdxType count = 0;
        while( count < len && !isFull() )
            put(buff[count++]);
        return count;
    }
    t_DataType  get()
    {
        auto retval = _buff[_tail];
        if( !isEmpty() )
        {
            _tail = incIdx(_tail);
            _count--;
        }
        return retval;
    }
    t_IdxType   get(t_DataType* dest,t_IdxType len)
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
        return len;
    }
    t_DataType  peek()    const
    {
        return _buff[_tail];
    }
    t_DataType  peekAt(t_IdxType idx) const
    {
        idx %= length();
        if( _tail < _head )
            return _buff[_tail+idx];
        if( _tail+idx < t_buffLen )
            return _buff[_tail+idx];
        return _buff[_tail+idx-t_buffLen];
    }
    bool 	    isEmpty() const
    {
        return _tail == _head;
    }
    bool 	    isFull()  const
    {
        return incIdx(_head) == _tail;
    }
    void	    clear(bool deepClean=false)
    {
        _tail = _head;
        _count = 0;
        if( deepClean )
            std::for_each(_buff,_buff+t_buffLen,[](t_DataType& item){ item = 0; });
    }
    void        remove(t_IdxType len,bool fromHead=false)
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
    }
    t_IdxType   length()  const
    {
        return _count;
    }
    t_IdxType   freeSpace() const
    {
        //t_buffLen - 1: the -1 is because this implementation to work must keep
        //one item without use
        return (t_buffLen-1) - length();
    }
    void setAbsoluteIdx(t_IdxType idx,t_DataType data)
    {
        idx = std::min(idx,t_IdxType(t_buffLen-1));
        _buff[idx] = data;
    }
    void setRelativeIdx(t_IdxType idx,const t_DataType& data)
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
    t_DataType getAbsoluteIdx(t_IdxType idx)
    {
        idx = std::min(idx,t_IdxType(t_buffLen-1));
        return _buff[idx];
    }
    t_DataType getRelativeIdx(t_IdxType idx)
    {
        idx %= length();
        if( _tail < _head )
            return _buff[_tail+idx];
        if( _tail+idx < t_buffLen )
            return _buff[_tail+idx];
        return _buff[_tail+idx-t_buffLen];
    }
    t_IdxType   getHead() const
    {
        return _head;
    }
    t_IdxType   getTail() const
    {
        return _tail;
    }
protected:
    t_IdxType   incIdx(t_IdxType idx) const
    {
        if( idx == t_buffLen-1 )
            return 0;
        return idx + 1;
    }
    t_IdxType   _count = 0;
protected:
    t_DataType  _buff[t_buffLen];
    t_IdxType   _tail = 0;
    t_IdxType   _head = 0;
};
}//namespace mcu
