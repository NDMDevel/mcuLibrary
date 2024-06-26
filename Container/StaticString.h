#pragma once

#include <cstdint>
#include <array>
#include <string_view>
#include <cstring>

namespace mcu
{

template<size_t N>
class StaticString
{
public:
    StaticString() { clear(); }
    auto clear() -> void { _str[0] = '\0'; _len = 0; }
    auto append(std::string_view sv) -> void
    {
        if( sv.length() > available() )
            return;
        strncat(_str.data()+_len,sv.begin(),sv.length());
        _len += sv.length();
    }
    auto append(char data) -> void
    {
        if( _len < N )
            _str[_len++] = data;
    }
    auto length() const -> size_t { return _len; }
    auto available() const -> size_t { return N - _len; }
    auto operator[](size_t idx) const -> char
    {
        if( idx < _len )
            return _str[idx];
        return '\0';
    }
    auto toStringView() const -> std::string_view
    {
        return std::string_view(_str.begin(),_len);
    }
    static constexpr auto capacity() -> size_t { return N; }
private:
    std::array<char,N> _str;
    size_t _len = 0;
};

}//namespace mcu
