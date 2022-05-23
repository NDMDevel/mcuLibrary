#pragma once

#include <cstdint>
#include <algorithm>
#include <functional>
#include "../Container/FifoBuffer.hpp"

#define DEBUG_SERIAL_DESKTOP
#ifdef DEBUG_SERIAL_DESKTOP
#include <iostream>
#endif

namespace mcu
{

//proto_header
template<
    typename t_IdxType, 
    t_IdxType t_rxLen,  
    t_IdxType t_txLen,
    int t_SerialId = 0>
class Serial
{
private:
    static constexpr t_IdxType INVALID_POS = ~t_IdxType(0);
    Serial() = delete;
public:
    //put to txFifo
    static void put(uint8_t data)
    {
        _txFifo.put(data);
    }
    static void put(uint8_t* data,t_IdxType len)
    {
        if( _txFifo.freeSpace() < len )
            return;
        std::for_each(data,data+len,[&](uint8_t data){_txFifo.put(data);});
    }
    //get from rxFifo
    static uint8_t get()
    {
        return _rxFifo.get();
    }
    static uint8_t peek(t_IdxType at=INVALID_POS)
    {
        if( at == INVALID_POS )
            return _rxFifo.peek();
        return _rxFifo.peekAt(at);
    }
    //space related
    static t_IdxType rxAvailable()
    {
        return _rxFifo.length();
    }
    static t_IdxType txAvailable()
    {
        return _txFifo.freeSpace();
    }
    static bool rxIsEmpty()
    {
        return _rxFifo.isEmpty();
    }
    static bool txIsEmpty()
    {
        return _txFifo.isEmpty();
    }
    //transfer methods
    static void putIntoRx(t_IdxType data)
    {
        _rxFifo.put(data);
    }
    static t_IdxType getFromTx()
    {
        return _txFifo.get();
    }
#ifdef DEBUG_SERIAL_DESKTOP
public: //debug
    static void showRx()
    {
        for( uint8_t idx=0 ; idx<_rxFifo.length() ; idx++ )
            std::cout << int(_rxFifo.peekAt(idx)) << std::endl;
    }
    static void showTx()
    {
        for( uint8_t idx=0 ; idx<_txFifo.length() ; idx++ )
            std::cout << int(_txFifo.peekAt(idx)) << std::endl;
    }
#endif
private:
    static FifoBuffer<uint8_t,t_IdxType,t_rxLen> _rxFifo;
    static FifoBuffer<uint8_t,t_IdxType,t_txLen> _txFifo;
};

template<typename t_IdxType,t_IdxType t_rxLen,t_IdxType t_txLen,int t_SerialId>
inline FifoBuffer<uint8_t,t_IdxType,t_rxLen> Serial<t_IdxType,t_rxLen,t_txLen,t_SerialId>::_rxFifo;
template<typename t_IdxType,t_IdxType t_rxLen,t_IdxType t_txLen,int t_SerialId>
inline FifoBuffer<uint8_t,t_IdxType,t_txLen> Serial<t_IdxType,t_rxLen,t_txLen,t_SerialId>::_txFifo;

}//namespace mcu
