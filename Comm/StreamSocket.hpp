#pragma once

#include "../Container/FifoRaw.hpp"
#include "../Timer/Timer.hpp"

#include <cstdint>
#include <tuple>

namespace mcu
{
template <
//    typename t_DataType,//=uint8_t
//    typename t_IdxType, //=uint8_t
    uint8_t t_rxLen,
    uint8_t t_txLen,
    typename t_Timer,
    typename t_Timer::TimerResolution t_eofTimeoutUs,    //10000us (10ms)
    uint8_t (*t_rxAvailable)(),
    void (*t_rxRead)(uint8_t*,uint8_t),
    uint8_t (*t_txFreeSpace)(),
    void (*t_txWrite)(const uint8_t*,uint8_t)>
class Serial
{
private:
    static_assert(std::is_base_of_v<TimerInterface<typename t_Timer::TimerResolution,t_Timer::Num,t_Timer::Den>,t_Timer>,"t_Timer must implement mcu::TimerInterface.");
private:
    enum class RxState
    {
        shutdown,
        init,
        waitEof,
        idle,
        read,
        readCompleted
    };
    enum class TxState
    {
        shutdown,
        init,
        waitEof,
        idle,
        flush,
    };
public://extported types/constants
    static constexpr auto eofTimeout = typename t_Timer::IncPeriod(t_eofTimeoutUs);
public:
    //-------------
    // RX hander
    //-------------
    bool      rxFrameAvailable() const
    {
        return _rxst == RxState::readCompleted;
    }
    uint8_t rxFrameLen() const
    {
        if( !rxFrameAvailable() )
            return 0;
        return _rxFifo.length();
    }
    uint8_t rxRead(uint8_t* buff,uint8_t len)
    {
        if( !rxFrameAvailable() )
            return 0;
        return _rxFifo.get(buff,len);
    }
    void rxTask()
    {
        if( _rxst == RxState::shutdown )
            return;
        if( _rxst == RxState::init )
        {
            const auto cleanRxHardware = [&]()
            {
                //this will free the rx input buffer
                while( t_rxAvailable() != 0 )
                {
                    uint8_t dummy;
                    t_rxRead(&dummy,1);
                }
            };
            cleanRxHardware();
            _rxTim.start();
            _rxst = RxState::waitEof;
            return;
        }
        if( _rxst == RxState::waitEof )
        {
            if( t_rxAvailable() != 0 )
                _rxst = RxState::init;
            if( _rxTim >= eofTimeout )
                _rxst = RxState::idle;
            return;
        }
        if( _rxst == RxState::idle )
        {
            if( t_rxAvailable() == 0 )
                return;
            //overflow: no space to store the input frame
            if( _rxFifo.freeSpace() <= 1 )
            {
                //goto RxState::init to ignore the frame
                _rxst = RxState::init;
                return;
            }
            _rxTim.start();
            _rxFrameLen = 0;
            
            //_rxFrameLenIdx points to the place where the FrameLen must be store
            _rxFrameLenIdx = _rxFifo.getHead();
            _rxFifo.put(_rxFrameLen);
            _rxst = RxState::read;
            return;
        }
        if( _rxst == RxState::read )
        {
            if( _rxTim >= eofTimeout )
            {
                
                _rxst = RxState::idle;
                return;
            }
            if( t_rxAvailable() != 0 )
            {
                while( t_rxAvailable() )
                {
                    uint8_t data;
                    t_rxRead(&data,1);
                    _rxFifo.put(data);
                    _rxFrameLen++;
                }
                _rxTim.start();
            }
            return;
        }
        if( _rxst == RxState::readCompleted )
        {
            if( !_rxFifo.isEmpty() )
                return;
            _rxst = RxState::idle;
            return;
        }
    }
    //-------------
    // TX hander
    //-------------
    uint8_t txFreeSpace() const
    {
        return _txFifo.freeSpace();
    }
    uint8_t txAppend(const uint8_t* buff,uint8_t len)
    {
        return _txFifo.put(buff,len);
    }
    void txFlush()
    {
        if( _txFifo.length() != 0 && !_txFlushActive )
            _txFlushActive = true;
    }
    void TxTask()
    {
        if( _txst == TxState::shutdown )
            return;
        if( _txst == TxState::init )
        {
            _txTim.start();
            _txst = TxState::waitEof;
            return;
        }
        if( _txst == TxState::waitEof )
        {
            if( _txTim < eofTimeout )
                return;
            _txst = TxState::idle;
            return;
        }
        if( _txst == TxState::idle )
        {
            if( !_txFlushActive )
                return;
            _txst = TxState::flush;
            return;
        }
        if( _txst == TxState::flush )
        {
            if( auto flushLen = std::min(t_txFreeSpace(),_txFifo.length()); flushLen != 0 )
            {
                while( flushLen-- )
                {
                    uint8_t data = _txFifo.get();
                    t_txWrite(&data,1);
                }
                if( _txFifo.isEmpty() )
                {
                    _txFlushActive = false;
                    _txst = TxState::init;
                }
            }
            return;
        }
    }
private:
    mcu::FifoRaw<uint8_t, uint8_t, t_rxLen> _rxFifo;
    mcu::FifoRaw<uint8_t, uint8_t, t_txLen> _txFifo;
    RxState _rxst = RxState::shutdown;
    TxState _txst = TxState::shutdown;
    t_Timer _rxTim;
    t_Timer _txTim;
    bool _txFlushActive = false;
    uint8_t _rxFrameLen;
    uint8_t _rxFrameCount;
    uint8_t _rxFrameLenIdx;
};
}//namespace mcu

namespace mcu_deprecated
{
    //frame format:
    // len (N bytes) byte1 byte2 byte3 ... len (N bytes) ...
template <
    typename t_LenType, //=uint8_t,
    t_LenType t_txLen,  //=32,
    t_LenType t_rxLen,  //=32>
    uint8_t (*t_rxAvailable)(),
    uint8_t (*t_txFreeSpace)(),
    bool (*t_txBusy)(),
    void (*t_txWrite)(const uint8_t *, t_LenType),
    uint8_t (*t_rxRead)()>
class StreamSocket
{
private:
    static constexpr t_LenType INVALID_IDX = ~t_LenType(-1);
    enum class StateFlush
    {
        sendLen,
        sendData,
        sendCRC
    };

public: //static methods
    template <typename T>
    static uint8_t getByte(T data, uint8_t bytePos)
    {
        static_assert(std::is_unsigned_v<T> && !std::is_same_v<T, bool>, "error: template type must be uintX_t");
        T mask = (T(0xFF) << (bytePos * 8));
        return uint8_t((mask & data) >> (bytePos * 8));
    }
    template <typename T>
    static void storeBytes(uint8_t *buff, uint8_t len, T data)
    {
        for (uint8_t idx = 0; idx < sizeof(T) && idx < len; idx++)
            buff[idx] = getByte(data, idx);
    }

private:
public:
    void task()
    {
        //reads rx
        if (auto len = std::min(t_rxAvailable(), _rxFifo.freeSpace()); len != 0)
            while (len--)
                _rxFifo.put(t_rxRead());
        if (_txFlush)
        {
            if (_stFlush == StateFlush::sendLen)
            {

                return;
            }
            if (_stFlush == StateFlush::sendData)
            {
                return;
            }
            if (_stFlush == StateFlush::sendCRC)
            {
                return;
            }
            //flush as much as it can
            t_LenType len = std::min(t_txFreeSpace(), _lenFlush);
            while (len)
            {
                uint8_t data = _txFifo.get();
                t_txWrite(&data, 1);
                len--;
                _lenFlush--;
            }
            if (_lenFlush == 0)
                _txFlush = false;
        }
    }
    t_LenType txFreeSpace() const
    {
        //(sizeof(t_LenType)+1) = LEN(sizeof(t_LenType)) + CRC(1 byte)
        return _txFifo.freeSpace() - (sizeof(t_LenType) + 1);
    }
    bool txBusy() const
    {
        return _txFlush;
    }
    void txAppend(uint8_t *buff, t_LenType len)
    {
        if (_txFlush)
            return;
        if (len > _txFifo.freeSpace())
            return;
        std::for_each(buff, buff + len, [&](uint8_t data) { _txFifo.put(data); });
    }
    void txFlush(t_LenType flushLen = INVALID_IDX)
    {
        if (_txFlush)
            return;
        //if no arg is passed, flush all _txFifo, otherwise
        //flush flushLen bytes
        if (flushLen == INVALID_IDX)
            _txFlush = _txFifo.length();
        else
            flushLen = std::min(flushLen, _txFifo.length());
        _txFlush = true;
    }
    t_LenType rxAvailable() const
    {
        return t_rxAvailable();
    }
    t_LenType rxRead(uint8_t *buff, t_LenType len)
    {
        t_LenType count = 0;
        while (count < len && !_rxFifo.isEmpty())
            buff[count++] = _rxFifo.get();
        return count;
    }
    class
    {
    private:
        enum class State
        {
            idle,
            sendLen,
            sendData,
            sendCRC
        };
    public:
        void task()
        {
            if( _st == State::idle )
                return;
            if( _st == State::sendLen )
            {

                return;
            }
            if( _st == State::sendData )
            {

                return;
            }
            if( _st == State::sendCRC )
            {

                return;
            }
        }
        void append(uint8_t *buff,t_LenType len)
        {

        }
    private:
        mcu::FifoBuffer<uint8_t, t_LenType, t_txLen>& txFifo;
        State _st = State::idle;
        t_LenType _txDataLen;
    } tx{.txFifo=_txFifo};

public:
    mcu::FifoBuffer<uint8_t, t_LenType, t_txLen> _txFifo;
    mcu::FifoBuffer<uint8_t, t_LenType, t_rxLen> _rxFifo;
    bool _txFlush = false;
    t_LenType _lenFlush = 0;
    StateFlush _stFlush = StateFlush::sendLen;
    //    t_LenType _len
};
} //namespace mcu_deprecated

/*
namespace mcu_deprecated
{

namespace sml = boost::sml;

template<typename t_DataType,
         typename t_IdxType,
         t_IdxType t_txBuffLen,
         t_IdxType t_rxBuffLen,
         t_IdxType t_sof>
class StreamSocket
{
private:
    using FifoBufferTx = FifoBuffer<t_DataType,t_IdxType,t_txBuffLen>;
    using FifoBufferRx = FifoBuffer<t_DataType,t_IdxType,t_rxBuffLen>;
private:
    struct fsm
    {
        //evets:
        struct update {};

        auto operator()() const
        {
            //guards:
            const auto guard = [](const auto& evt)
            {
                return true;
            };
            //actions
            const auto action = []()
            {

            };
            using namespace sml;
            return make_transition_table(
*"init"_s   + event<update> [ guard ] / action = "state"_s
            );
        }
    };
public:
    StreamSocket(FifoBufferTx& txBuffer,FifoBufferRx& rxBuffer)
        : _txBuffer(txBuffer)
        , _rxBuffer(rxBuffer)
    {
        _txBuffer.clear();
        _rxBuffer.clear();
    }
    void  put(const t_DataType* buff,t_IdxType buffLen);
    void  get(      t_DataType* buff,t_IdxType buffLen);
    const t_DataType& peek() const;
    const t_DataType& peekAt(t_IdxType pos) const;
    t_IdxType  txAvailable() const;
    t_IdxType  rxAvailable() const{ return _rxCount; }

    void txTask();
    class
    {
    private:
        enum State
        {
            shutdown,
            waitSof,
            validateSof,
            readData
        };
    public:
        void start(){ if( _st == State::shutdown ) _st = State::waitSof; }
        void stop() { _st = State::shutdown; }
        void operator()()
        {
            const auto isSof = [&]() -> bool
            {
                return _rxBuffer.get() == t_sof;
            };
            if( _st == State::shutdown )
                return;
            if( _st == State::waitSof )
            {
                if( isSof() )
                    _st = State::validateSof;
                return;
            }
            if( _st == State::validateSof )
            {
                if( _rxBuffer.peek() == t_sof )
                {
                    _rxBuffer.get();
                    _st = State::waitSof;
                    return;
                }
                return;
            }
            if( _st == State::readData )
            {
                return;
            }
        }
    private:
        State _st = State::waitSof;
//        FifoBufferTx& _txBuffer;
//        FifoBufferRx& _rxBuffer;
    } rxTask;

private:
    FifoBufferTx& _txBuffer;
    FifoBufferRx& _rxBuffer;
    t_IdxType _rxCount;
};

}//namespace mcu_deprecated

*/
