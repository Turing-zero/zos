#include <iostream>
#include "zos/log.h"
template<typename CharT, std::size_t N>
struct fixed_string{
    CharT data[N]{};
    constexpr fixed_string(const CharT (&str)[N]){ std::copy_n(str, N, data); }
};

template<fixed_string Str>
struct FixedStringContainer{
    void print() { fmt::print("{}",Str.data); }
};

int main(){
    constexpr fixed_string fs{"Hello, C++20\n"};
    FixedStringContainer<fs> fc{};
    fc.print();
}