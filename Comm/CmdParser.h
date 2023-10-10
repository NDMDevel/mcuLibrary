#pragma once

#include "../Container/FifoBuffer.hpp"
#include "../Utils/TypeUtils.hpp"
#include "../Container/VLItemFifo.h"
#include <array>
#include <string_view>
#include <utility>

namespace mcu {

/* This implementation has the following limitations:

----------- LIMITATION 1 ----------------------------
1- does not support nested strings

   Example 1:

some_command arg1 "arg2 with some spaces" arg3

The parsing result:
command: some_command
argument1: arg1
argument2: "arg2 with some spaces"
argument3: arg3

    Exampel 2:

some_command arg1 "arg2 with "some " spaces" arg3

The parsing result:
command: some_command
argument1: arg1
argument2: "arg2 with "some
argument3: " spaces"
argument4: arg3

----------- LIMITATION 2 ----------------------------
2- if an EOF is found inside a string it will be not ignored and considered
   as the end of the frame

    Example, EOF = \r

some_command arg1 "arg2 with\r eof inside string" arg3

The parsing result:
first command: some_command
argument1: arg1
argument2: "arg2 with

second command: eof
argument1: inside
argument2: string" arg3

----------- LIMITATION 3 ----------------------------
3- if the input stream (the bytes sended to this parser) is too long, the parser
   will not report the overflow and will restart after the next EOF

    Example, EOF = \r, max len 32:

                                   all these bytes are ignored
                               ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
some_command arg1 arg2 arg3 "argument very very large...." arg4 arg5\r other_command arg\r
                              ^                                      ^
                              |                                      |
                              +--> here the input buffer overflows   |
                                                                     |
                                           here the parser resets <--+
The parsing result:
command: other_command
argument1: arg1

*/


/*
<-- Buffer length: 32 bytes --->
0123456789ABCDEF0123456789ABCDEF
COM ARG1 ARG2 ARG3 EOF..........

Example:
echo "this is one ARGUMENT with spaces" ARG2\r
.echo....."this is one ARGUMENT with spaces"...ARG2...\r
*/

enum class CmdParserRetType: uint8_t
{
    finished_ok,
    finished_nok,
    not_finished
};

template<   char   t_separator     , // = ' '
            char   t_eofMarker     , // = '\r
            size_t t_frameMaxLen   , // = 128
            size_t t_commandsCount ,
            const std::array<std::pair<std::string_view,CmdParserRetType(*)(VLItemFifo<t_frameMaxLen>&)>,t_commandsCount>& t_commandProcessors>
class CmdParser
{
private:
    template<const std::array<std::pair<std::string_view,CmdParserRetType(*)(VLItemFifo<t_frameMaxLen>&)>,t_commandsCount>& arg>
    static consteval bool check_all_different()
    {
        for( size_t i=0 ; i<arg.size() ; i++ )
            for( size_t j=i+1 ; j<arg.size() ; j++ )
                if( arg[i].first == arg[j].first )
                    return false;
        //        for( auto item : arg )
        //            if( item.first != 0 )
        //                return false;
        return true;
    }
    static_assert( check_all_different<t_commandProcessors>() , "command tag repeated" );
private:
    using BufferType = FifoBuffer<uint8_t,t_frameMaxLen>;
    enum class TaskState
    {
        shutdown,
        readUntilEof,
        parseFrame,
        identifyCommand,
        invokeCommand
    };
public:
    using commandProcessorPrototype = CmdParserRetType(*)(VLItemFifo<t_frameMaxLen>&);
public:
    void start()
    {
        if( _st == TaskState::shutdown )
        {
            _st = TaskState::readUntilEof;
            _eofIdx = 0;
            _pushCnt = 0;
            _pushWaitEof = false;
            _rxBuff.clear();
        }
    }
    void operator()()
    {
        if( _st == TaskState::shutdown )
            return;
        if( _st == TaskState::readUntilEof )
        {
            if( _eofIdx >= _rxBuff.length() )
                return;
            auto cnt = std::min(_rxBuff.length()-_eofIdx,1);
            while( cnt-- )
            {
                if( _rxBuff[_eofIdx] == t_eofMarker )
                {
                    _st = TaskState::parseFrame;
                    return;
                }
                _eofIdx++;
            }
            return;
        }
        if( _st == TaskState::parseFrame )
        {
            if( _eofIdx == 0 )
            {
                _rxBuff.remove(1);
                _st = TaskState::readUntilEof;
                return;
            }
//            std::cout << " --> frame parsing" << std::endl;
            _command.clear();
            //string   _______________-----------------_____________
            //capture  ___-----_-----_------------------_-----------
            //com value:  write value "this is a string" other_value

            //CAPTURE logic:
            //capture toogle if (capture==true && separator)
            //capture toogle if (capture==false && !separator)
            //toggle = !C^!S v C^S
            //toggle = !(CvS) v C^S

            //STRING logic:
            //capture toogle if ( data == " )
            bool string = false;
            bool capture = false;
            bool prevCapture = capture;
            bool parsingOk = true;
            for( typename BufferType::IdxType i=0 ; i<_eofIdx ; i++ )
            {
                if( _command.freeSpace() == 0 )
                {
                    parsingOk = false;
                    break;
                }
                if( !string )
                {
                    bool C  = capture;
                    bool S  = isSeparator(_rxBuff[i]);
                    if( !(C || S) || (C && S) )
                    {
                        capture = !capture;
                    }
                }
                bool SC = _rxBuff[i] == '"';
                if( SC )
                    string = !string;
                if( capture )
                {
                    auto data = _rxBuff[i];
                    if( prevCapture == false )
                        _command.pushItem(data);
                    else
                        _command.appendByteToItem(data);
                }
                prevCapture = capture;
            }
            //Now the frame is removed from the input buffer.
            //The removal of the frame is done by removing first
            //_eofIdx bytes and then one more instead of
            //removing _eofIdx+1. It is done in this way to avoid a potencial
            //overflow by computing _eofIdx+1 when _eofIdx is at its max value (2^N)-1
            _rxBuff.remove(_eofIdx);
            _rxBuff.remove(1);
            _eofIdx = 0;
//            std::cout << "**** _command in parsing ****" << std::endl;
//            _command.template print_internals<char,false>();
            if( !parsingOk )
            {
//                std::cout << "{command overflow}" << std::endl;
                _st = TaskState::readUntilEof;
                return;
            }
//            std::cout << "<command ok>" << std::endl;
            _st = TaskState::identifyCommand;
            return;
        }
        if( _st == TaskState::identifyCommand )
        {
            if( auto command = _command.peekStringAt(0); command.has_value() )
            {
                const auto findCommandProcessor = [&](std::string_view strv)->commandProcessorPrototype
                {
                    auto found = std::find_if(t_commandProcessors.begin(),
                                              t_commandProcessors.end(),
                                              [&](auto value)->bool
                                              {
                                                  return strv == value.first;
                                              });
                    if( found == t_commandProcessors.end() )
                        return nullptr;
                    return found->second;
                };
                using VLIdx = typename decltype(_command)::IdxType;
                auto strv = command.value();
//                std::cout << "command: " << strv << std::endl;
                _processor = findCommandProcessor(strv);
/*                for( auto item : t_commandProcessors )
                {
                    if( strlen(item.first) != strv.size() )
                        continue;
                    bool comFound = false;
                    for( VLIdx j=0 ; j<strv.size() ; j++ )
                    {
                        if( strv[j] != item.first[j] )
                            break;
                        comFound = true;
                    }
                    if( comFound )
                    {
                        _processor = item.second;
                        break;
                    }
                }*/
                if( _processor == nullptr )
                {
                    _st = TaskState::readUntilEof;
                    return;
                }
            }
            _st = TaskState::invokeCommand;
            return;
        }
        if( _st == TaskState::invokeCommand )
        {
//            _command.template print_internals<char,false>();
            if( _processor(_command) == CmdParserRetType::not_finished )
                return;
            _st = TaskState::readUntilEof;
            return;
        }
    }
    void pushData(uint8_t data)
    {
        if( _rxBuff.isFull() )
        {
//            std::cout << "[overflow detected]" << std::endl;
            if( _pushCnt == _rxBuff.length() )
                _eofIdx = 0;
            _rxBuff.remove(_pushCnt,true);
            _pushCnt = 0;
            _pushWaitEof = true;
//            for( size_t i=0 ; i<_rxBuff.length() ; i++ )
//                std::cout << char(_rxBuff[i]);
//            std::cout << std::endl;
        }
        if( _pushWaitEof )
        {
            if( data == t_eofMarker )
                _pushWaitEof = false;
            return;
        }
        _pushCnt++;
        _rxBuff.put(data);
        if( data == t_eofMarker )
            _pushCnt = 0;
//        for( size_t i=0 ; i<_rxBuff.length() ; i++ )
//            std::cout << char(_rxBuff[i]);
//        std::cout << std::endl;
    }
private:
    bool isSeparator(uint8_t data) const{ return (data==t_separator) || (t_separator==' ' && data=='\t'); }
private:
    BufferType _rxBuff;
    VLItemFifo<t_frameMaxLen> _command;
    commandProcessorPrototype _processor;
    typename BufferType::IdxType _eofIdx;
    TaskState _st = TaskState::shutdown;
    typename BufferType::IdxType _pushCnt = 0;
    bool _pushWaitEof = false;
};
} //namespace mcu
