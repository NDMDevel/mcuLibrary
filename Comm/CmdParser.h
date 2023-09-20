#pragma once

#include "../Container/FifoBuffer.hpp"

namespace mcu {

enum class CmdParserRetType: uint8_t
{

};
/*
<-- Buffer length: 32 bytes --->
0123456789ABCDEF0123456789ABCDEF
COM ARG1 ARG2 ARG3 EOF..........
....ARG1 ARG2 ARG3 EOF349E3.....

349E3
^------- argument count
349E3
 ^------ index of first arg
349E3
  ^------ index of second arg
349E3
   ^------ index of thierd arg
349E3
    ^------ index of EOF

Example:
echo "this is one \"ARGUMENT\" with\r spaces" ARG2\r
*/
template<size_t t_BufferLen,char t_eofMarker='\r',char t_separator=' '>
class CmdParser
{
public:


private:
    FifoBuffer<uint8_t,t_BufferLen> _rxBuff;
};

} //namespace mcu
