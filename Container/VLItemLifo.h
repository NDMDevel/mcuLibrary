#pragma once
#include "../Utils/TypeUtils.hpp"
#include "../Utils/SerializableT.hpp"
#include <array>
#include <span>
#include <optional>
#include <string_view>

#ifdef DEBUG_VLITEMLIFO
#include <iostream>
#endif

/*
VLItemLifo: Variable Length Item Lifo (last in first out)
Buffer that can store items of different sizes

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
class VLItemLifo
{
public:
    using IdxType = fit_combinations_t<t_buffLen>;
private:
    static_assert( t_buffLen > 3*sizeof(IdxType) , "t_buffLen must be at least 4*sizeof(IdxType) to be usefull" );
public:
    VLItemLifo(){ clear(); }
    void clear()
    {
        len() = 0;
        tos() = 0;
    }
    IdxType itemsCount() const { return len(); }
    std::optional<std::string_view> peekStringAt(IdxType itemIdx) const
    {
        if( auto ospan=peekSpanAt(itemIdx); ospan.has_value() )
        {
            auto span = ospan.value();
            std::string_view strv(reinterpret_cast<const char*>(span.data()),span.size());
            return strv;
        }
        return std::nullopt;
    }
    std::optional<std::span<const uint8_t>> peekSpanAt(IdxType itemIdx) const
    {
        const auto startIdx = [&](IdxType itemIdx) -> IdxType
        {
            return _buff[t_buffLen-(3+itemIdx)*sizeof(IdxType)];
        };
        auto iC = itemsCount();
        if( itemIdx >= itemsCount() )
            return std::nullopt;
        IdxType startIndex = startIdx(itemIdx);
        IdxType len = 0;
        if( itemIdx == itemsCount()-1 )
            len = tos() - startIdx(itemIdx);
        else
        {
            if( itemIdx == 0 )
                len = startIdx(itemIdx+1);
            else
                len = startIdx(itemIdx) - startIdx(itemIdx-1);
        }
        std::span span{_buff.data()+startIndex,len};
//        const ItemInfo item{.data = _buff.data()+startIndex , .len = len};
        return {span};
    }
/*    std::optional<ItemInfo> peekAt(IdxType itemIdx) const
    {
        const auto startIdx = [&](IdxType itemIdx) -> IdxType
        {
            return _buff[t_buffLen-3-itemIdx];
        };
        auto iC = itemsCount();
        if( itemIdx >= itemsCount() )
            return std::nullopt;
        IdxType startIndex = startIdx(itemIdx);
        IdxType len = 0;
        if( itemIdx == itemsCount()-1 )
            len = tos() - startIdx(itemIdx);
        else
        {
            if( itemIdx == 0 )
                len = startIdx(itemIdx+1);
            else
                len = startIdx(itemIdx) - startIdx(itemIdx-1);
        }
        std::span span{_buff.data()+startIndex,len};
        const ItemInfo item{.data = _buff.data()+startIndex , .len = len};
        return {item};
    }*/
    template<typename T>
    bool pushItem(const T& data)
    {
        if( freeSpace() < sizeof(T) )
            return false;
        SerializableT<T> sdata(data);
        auto startIdx = tos();
        nextEmptyStartIdx() = startIdx;
        for( IdxType i=0 ; i<sizeof(T) ; i++ )
        {
            _buff[startIdx++] = sdata.raw[i];
        }
        tos() += sizeof(T);
        len()++;
        return true;
    }
    bool appendByteToItem(uint8_t data)
    {
        if( freeSpace() == 0 )
            return false;
        if( itemsCount() == 0 )
        {
            auto startIdx = tos();
            nextEmptyStartIdx() = startIdx;
            _buff[startIdx] = data;
            tos() += sizeof(data);
            len()++;
            return true;
        }
        _buff[tos()++] = data;
        return true;
    }
    template<typename T>
    std::optional<T> popItem()
    {
        const auto lastStartIdx = [&]() -> IdxType
        {
            return *reinterpret_cast<const IdxType*>(_buff.data() + t_buffLen-(2+len())*sizeof(IdxType));
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
    bool isEmpty() const { return itemsCount() == 0; }
    IdxType freeSpace() const
    {
        if( tos()+len()+2*sizeof(IdxType) == t_buffLen )
            return 0;
        return t_buffLen-2*sizeof(IdxType)-len()-tos()-1*sizeof(IdxType);
    }
#ifdef DEBUG_VLITEMLIFO
    template<typename T=int,bool separateWhitSpace=true>
    void print_internals() const

    {
        for( size_t i=0 ; i<_buff.size() ; i++ )
        {
            if( i >= tos() )
            {
                std::cout <</* std::hex <<*/ int(_buff[i]) << " ";
            }
            else
            {
                if( separateWhitSpace )
                    std::cout <</* std::hex <<*/ T(_buff[i]) << " ";
                else
                    std::cout <</* std::hex <<*/ T(_buff[i]);
            }
        }
        std::cout << std::endl;
    }
#endif
private:
    IdxType  tos() const { return *reinterpret_cast<const IdxType*>(_buff.data()+t_buffLen-2*sizeof(IdxType)); }
    IdxType& tos()       { return *reinterpret_cast<      IdxType*>(_buff.data()+t_buffLen-2*sizeof(IdxType)); }
    IdxType  len() const { return *reinterpret_cast<const IdxType*>(_buff.data()+t_buffLen-1*sizeof(IdxType)); }
    IdxType& len()       { return *reinterpret_cast<      IdxType*>(_buff.data()+t_buffLen-1*sizeof(IdxType)); }
    IdxType  nextEmptyStartIdx() const { return *reinterpret_cast<const IdxType*>(_buff.data()+t_buffLen-(3+len())*sizeof(IdxType)); }
    IdxType& nextEmptyStartIdx()       { return *reinterpret_cast<      IdxType*>(_buff.data()+t_buffLen-(3+len())*sizeof(IdxType)); }
private:
    std::array<uint8_t,t_buffLen> _buff;
};

} //namespace mcu
