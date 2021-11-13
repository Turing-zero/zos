#ifndef __ZOS_LOG_H__
#define __ZOS_LOG_H__
#include <source_location>
#include <fmt/core.h>
#include <fmt/chrono.h>
#include <string_view>
#include <ctime>
namespace zos{
namespace __impl{
template<typename... Ts>
struct __Log{
    __Log(std::string_view s,Ts&&... ts,const std::source_location& lo=std::source_location::current()){
        print_timestamp();
        std::string total_filename{lo.file_name()};
        auto found = total_filename.find_last_of('/');
        std::string filename{found!=std::string::npos ? total_filename.substr(found+1) : std::move(total_filename)};
        // fmt::print("{}({}:{})`{}`:",filename,lo.line(),lo.column(),lo.function_name());
        fmt::print("{}({}:{}):",filename,lo.line(),lo.column());
        fmt::print(s,ts...);
    }
    static void print_timestamp(){
        auto milli = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()%1000000;
        fmt::print("({:%Y-%m-%d %H:%M:%S}.{:0<6d}):",fmt::localtime(std::time(0)),milli);
    }
};
template<typename... Ts> __Log(std::string_view s,Ts&&... ts) -> __Log<Ts...>;
} // namespace zos::impl
template<typename... Ts>
using log = __impl::__Log<Ts...>;
} // namespace zos
#endif // __ZOS_LOG_H__