#pragma once
#include "../Utils/TypeUtils.hpp"
#include "../Utils/SerializableT.hpp"
#include <array>
#include <optional>

#ifdef DEBUG_VLITEMFIFO
#include <iostream>
#endif

/*
VLItemFifo: Variable Length Item Fifo
Buffer that store items of different sizes

0123456789ABCDEF0123456789ABCDEF
data0data1data2............A50F3
                           ^^^^^
                           |||||
                           ||||+-- count of items
                           |||+--- TOS: top of stack
                           ||+---- start idx of first item
                           |+----- start idx of second item
                           +------ start idx of thierd item
*/

namespace mcu {

template<size_t t_buffLen>
class VLItemFifo
{
public:
    using IdxType = fit_combinations_t<t_buffLen>;
    struct ItemInfo
    {
        const uint8_t *data;
        const IdxType len;
    };
private:
    static_assert( t_buffLen > 3 , "t_buffLen must be at least 4 to be usefull" );
public:
    VLItemFifo()
    {
        len() = 0;
        tos() = 0;
    }
    IdxType length() const { return len(); }
    std::optional<ItemInfo> peekAt(IdxType itemIdx) const
    {
        const auto startIdx = [&](IdxType itemIdx) -> IdxType
        {
            return _buff[t_buffLen-3-itemIdx];
        };
        if( itemIdx >= length() )
            return std::nullopt;
        IdxType startIndex = startIdx(itemIdx);
        IdxType len = 0;
        if( itemIdx == length()-1 )
            len = tos() - startIdx(itemIdx);
        else
            len = startIdx(itemIdx-1) - startIdx(itemIdx);
        const ItemInfo item{.data = _buff.data()+startIndex , .len = len};
        return {item};
    }
    template<typename T>
    bool push(const T& data)
    {
        if( freeSpace() < sizeof(T) )
            return false;
        SerializableT<T> sdata(data);
        auto startIdx = tos();
        nextEmptyStartIdx() = startIdx;
        for( IdxType i=0 ; i<sizeof(T) ; i++ )
            _buff[startIdx++] = sdata.raw[i];
        tos() += sizeof(T);
        len()++;
        return true;
    }
    template<typename T>
    std::optional<T> pop()
    {
        const auto lastStartIdx = [&]() -> IdxType
        {
            return _buff[t_buffLen-2-len()];
        };
        if( isEmpty() )
            return std::nullopt;
        auto startIdx = lastStartIdx();
        IdxType len = tos() - startIdx;
        if( len != sizeof(T) )
            return std::nullopt;
        SerializableT<T> aux;
        aux.copyFrom(_buff.data()+startIdx);
        (this->len())--;
        tos() -= sizeof(T);
        return {aux.value};
    }
    bool isEmpty() const { return length() == 0; }
    IdxType freeSpace() const
    {
        if( tos()+len()+2 == t_buffLen )
            return 0;
        return t_buffLen-2-len()-tos()-1;
    }
#ifdef DEBUG_VLITEMFIFO
    void print_internals() const
    {
        for( size_t i=0 ; i<_buff.size() ; i++ )
            std::cout << std::hex << int(_buff[i]) << " ";
        std::cout << std::endl;
    }
#endif
//private:
//    const uint8_t* addressAt(IdxType idx) const{ return _buff.data()+lastStartIdx(idx); }
    IdxType  tos() const { return *(_buff.data()+t_buffLen-2); }
    IdxType& tos()       { return *(_buff.data()+t_buffLen-2); }
    IdxType  len() const { return *(_buff.data()+t_buffLen-1); }
    IdxType& len()       { return *(_buff.data()+t_buffLen-1); }
    IdxType  nextEmptyStartIdx() const { return _buff[t_buffLen-3-len()]; }
    IdxType& nextEmptyStartIdx()       { return _buff[t_buffLen-3-len()]; }
private:
    std::array<uint8_t,t_buffLen> _buff;
};

} //namespace mcu
