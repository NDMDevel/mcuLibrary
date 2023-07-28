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

template<size_t t_rx_len,
         size_t t_tx_len,
         typename t_Timer,
         typename t_Timer::TimerResolution t_eofTimeout,
         uint32_t (*t_rx_available)(),
         uint8_t  (*t_rx_read)(),
         bool     (*t_tx_ready)(),
         void     (*t_tx_write)(const uint8_t*,uint32_t len)>
class SerialTranseiver
{
private:
    static_assert( t_rx_available   != nullptr , "template parameter t_rx_available can not be nullptr");
    static_assert( t_rx_read        != nullptr , "template parameter t_rx_read can not be nullptr");
    static_assert( t_tx_ready       != nullptr , "template parameter t_tx_ready can not be nullptr");
    static_assert( t_tx_write       != nullptr , "template parameter t_tx_write can not be nullptr");
public:
    using rx_idx_t = mcu::fit_value_t<t_rx_len>;
public:
    //rx driver
    auto rx_frame_available() -> bool { return false; }
    auto rx_frame_discard() -> void {}
    auto rx_frame_peek(rx_idx_t idx = 0) const -> uint8_t {}
    auto rx_task() -> void{}
    //tx driver
    auto tx_send_then_eof(const uint8_t* buff,uint32_t len) -> void{}
    auto tx_send(const uint8_t* buff,uint32_t len) -> void {}
    auto tx_ready() -> bool { return t_tx_ready(); }
    auto tx_task() -> void{}
private:

};
/*
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
    static void flushTx()
    {
        
    }
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
    static FifoBuffer<uint8_t,t_rxLen> _rxFifo;
    static FifoBuffer<uint8_t,t_txLen> _txFifo;
};

template<typename t_IdxType,t_IdxType t_rxLen,t_IdxType t_txLen,int t_SerialId>
inline FifoBuffer<uint8_t,t_IdxType,t_rxLen> Serial<t_IdxType,t_rxLen,t_txLen,t_SerialId>::_rxFifo;
template<typename t_IdxType,t_IdxType t_rxLen,t_IdxType t_txLen,int t_SerialId>
inline FifoBuffer<uint8_t,t_IdxType,t_txLen> Serial<t_IdxType,t_rxLen,t_txLen,t_SerialId>::_txFifo;
*/
}//namespace mcu
