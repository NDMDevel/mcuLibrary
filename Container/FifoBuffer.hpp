#pragma once

#include <algorithm>
#include <cstdint>
#include "../Utils/TypeUtils.hpp"

namespace mcu
{

template<   typename    t_DataType,
            size_t      t_buffLen,
            bool        t_override = false>
class FifoBuffer
{
public:
    static constexpr auto maxLen = t_buffLen;
    using IdxType = fit_combinations_t<t_buffLen>;
public:
                FifoBuffer(bool deepClean=false){ clear(deepClean); }
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
        _isFull = (_head == _tail);
    }
    IdxType     put(const t_DataType* buff,IdxType len)
    {
        IdxType count = 0;
        while( count < len && !isFull() )
        {
            if( buff != nullptr )
                put(buff[count]);
            else
                put(0);
            count++;
        }
        return count;
    }
    t_DataType  get()
    {
        auto retval = _buff[_tail];
        if( !isEmpty() )
            _tail = incIdx(_tail);
        _isFull = false;
        return retval;
    }
    IdxType     get(t_DataType* dest,IdxType len)
    {
        len = std::min(length(),len);
        if( len != 0 )
            return;
        for( t_DataType idx=0 ; idx<len ; idx++ )
        {
            if( dest != nullptr )
                dest[idx] = _buff[_tail];
            _tail = incIdx(_tail);
        }
        return len;
    }
    t_DataType  peek()    const
    {
        return _buff[_tail];
    }
    t_DataType  peekAt(IdxType idx) const
    {
        return (*this)[idx];
    }
    bool 	    isEmpty() const
    {
        return (_tail == _head) && !_isFull;
    }
    bool 	    isFull()  const
    {
        return _isFull;
    }
    void	    clear(bool deepClean=false)
    {
        _tail = _head;
        _isFull = false;
        if( deepClean )
            for( auto& item : _buff )
                item = 0;
    }
    void        remove(IdxType len,bool fromHead=false)
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
        _isFull = false;
    }
    IdxType     length()  const
    {
        if( _isFull )
            return t_buffLen;
        if( _tail <= _head )
            return _head - _tail;
        return t_buffLen - _tail + _head;
    }
    IdxType     freeSpace() const
    {
        return t_buffLen - length();
    }
protected:
    IdxType     incIdx(IdxType idx) const
    {
        if( idx == t_buffLen-1 )
            return 0;
        return idx + 1;
    }
protected:
    t_DataType  _buff[t_buffLen];
    IdxType     _tail   = 0;
    IdxType     _head   = 0;
    bool        _isFull = false;
};


template<   typename    t_DataType,
            size_t      t_buffLen,
            bool        t_override = false>
class FifoRaw
{
public:
    using IdxType = decltype(fit_combinations_t<t_buffLen>());
    static constexpr IdxType maxLen(){ return t_buffLen; }
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
        _isFull = (_head == _tail);
    }
    IdxType     put(const t_DataType* buff,IdxType len)
    {
        IdxType count = 0;
        while( count < len && !isFull() )
        {
            if( buff != nullptr )
                put(buff[count]);
            else
                put(0);
            count++;
        }
        return count;
    }
    t_DataType  get()
    {
        auto retval = _buff[_tail];
        if( !isEmpty() )
            _tail = incIdx(_tail);
        _isFull = false;
        return retval;
    }
    IdxType     get(t_DataType* dest,IdxType len)
    {
        len = std::min(length(),len);
        if( len != 0 )
            return;
        for( t_DataType idx=0 ; idx<len ; idx++ )
        {
            if( dest != nullptr )
                dest[idx] = _buff[_tail];
            _tail = incIdx(_tail);
        }
        return len;
    }
    t_DataType  peek()    const
    {
        return _buff[_tail];
    }
    t_DataType  peekAt(IdxType idx) const
    {
        return (*this)[idx];
    }
    bool 	    isEmpty() const
    {
        return (_tail == _head) && !_isFull;
    }
    bool 	    isFull()  const
    {
        return _isFull;
    }
    void	    clear(bool deepClean=false)
    {
        _tail = _head;
        _isFull = false;
        if( deepClean )
            for( auto& item : _buff )
                item = 0;
    }
    void        remove(IdxType len,bool fromHead=false)
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
        _isFull = false;
    }
    IdxType     length()  const
    {
        if( _isFull )
            return t_buffLen;
        if( _tail <= _head )
            return _head - _tail;
        return t_buffLen - _tail + _head;
    }
    IdxType     freeSpace() const
    {
        return t_buffLen - length();
    }
    void        setDataAtAbsoluteIdx(IdxType idx,t_DataType data)
    {
        idx = std::min(idx,IdxType(t_buffLen-1));
        _buff[idx] = data;
    }
    void        setDataAtRelativeIdx(IdxType idx,const t_DataType& data)
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
    IdxType     setDataAtRelativeIdx(IdxType idx,const t_DataType* data,IdxType len)
    {
        if( idx >= length() )
            return 0;
        len = std::min(len,IdxType(length()-idx));
        auto ret = 0;
        if( len == 0 )
            return 0;
        if( _tail < _head )
        {
            for( IdxType i=0 ; i<len ; i++ )
                _buff[_tail+idx+i] = data[i];
            return len;
        }
        IdxType i = 0;
        if( t_buffLen-_tail > idx )
        {
            IdxType cnt = t_buffLen - (_tail + idx);
            for( IdxType i=0 ; i<cnt ; i++ )
                _buff[_tail+idx+i] = *data++;
            len -= cnt;
            ret += cnt;
        }
        else
        {
            idx -= t_buffLen-_tail;
            i = idx;
        }
        while( len-- && _head>i )
        {
            _buff[i++] = *data++;
            ret++;
        }
        return ret;
    }
    t_DataType  getDataAtAbsoluteIdx(IdxType idx)
    {
        idx = std::min(idx,IdxType(t_buffLen-1));
        return _buff[idx];
    }
    t_DataType  getDataAtRelativeIdx(IdxType idx)
    {
        idx %= length();
        if( _tail < _head )
            return _buff[_tail+idx];
        if( _tail+idx < t_buffLen )
            return _buff[_tail+idx];
        return _buff[_tail+idx-t_buffLen];
    }
    IdxType     getHead() const
    {
        return _head;
    }
    IdxType     getTail() const
    {
        return _tail;
    }
    t_DataType& operator[](IdxType idx)
    {
        if( length() == 0 )
            return _buff[_head];
        idx %= length();
        if( _tail < _head )
            return _buff[_tail+idx];
        if( _tail+idx < t_buffLen )
            return _buff[_tail+idx];
        return _buff[_tail+idx-t_buffLen];
    }
    t_DataType  operator[](IdxType idx) const
    {
        if( length() == 0 )
            return _buff[_head];
        idx %= length();
        if( _tail < _head )
            return _buff[_tail+idx];
        if( _tail+idx < t_buffLen )
            return _buff[_tail+idx];
        return _buff[_tail+idx-t_buffLen];
    }
protected:
    IdxType     incIdx(IdxType idx) const
    {
        if( idx == t_buffLen-1 )
            return 0;
        return idx + 1;
    }
protected:
    t_DataType  _buff[t_buffLen];
    IdxType     _tail   = 0;
    IdxType     _head   = 0;
    bool        _isFull = false;
};

}//namespace mcu
