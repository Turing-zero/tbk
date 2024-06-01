#ifndef __TBK_LOG_H__
#define __TBK_LOG_H__
#include <source_location>
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <string_view>
#include <ctime>
#include <mutex>
#include "tbk/utils/singleton.h"
#include "tbk/meta.h"
namespace tbk{
// template <typename... Args>
// auto format(fmt::format_string<Args...> s,Args&&... args) {
//     return fmt::format(s,std::forward<Args>(args)...);
// }
namespace __impl{
typedef tbk::Singleton<std::mutex> LogMutex;
template<typename... Ts>
struct __Log{
    __Log(fmt::text_style style,std::string_view s,const Ts&... ts,const std::source_location& lo=std::source_location::current()){
        std::string total_filename{lo.file_name()};
        auto found = total_filename.find_last_of('/');
        std::string filename{found!=std::string::npos ? total_filename.substr(found+1) : std::move(total_filename)};
        // fmt::print("{}({}:{})`{}`:",filename,lo.line(),lo.column(),lo.function_name());
        {
            std::scoped_lock<std::mutex> lock(*LogMutex::_());
            print_timestamp(style);
            fmt::print(style,"{}({}):",filename,lo.line());
            fmt::print(style,s,ts...);
            fflush(stdout);
        }
    }
    static void print_timestamp(fmt::text_style style){
        auto milli = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()%1000000;
        fmt::print(style,"({:%Y-%m-%d %H:%M:%S}.{:0<6d}):",fmt::localtime(std::time(0)),milli);
    }
};
} // namespace tbk::impl
const auto STYLE_LOG = fmt::fg(fmt::color::sea_green);
const auto STYLE_STATUS = fmt::fg(fmt::color::green_yellow);
const auto STYLE_INFO = fmt::fg(fmt::color::royal_blue);
const auto STYLE_WARNING = fmt::fg(fmt::color::yellow);
const auto STYLE_ERROR = fmt::fg(fmt::color::red);
template<typename... Ts>
struct log: tbk::__impl::__Log<Ts...>{
    log(std::string_view s,const Ts&... ts,const std::source_location& lo=std::source_location::current()):tbk::__impl::__Log<Ts...>(STYLE_LOG,s,ts...,lo){}
    log(fmt::text_style style,std::string_view s,const Ts&... ts,const std::source_location& lo=std::source_location::current()):tbk::__impl::__Log<Ts...>(style,s,ts...,lo){}
};
template<typename... Ts>
struct status: tbk::__impl::__Log<Ts...>{
    status(std::string_view s,const Ts&... ts,const std::source_location& lo=std::source_location::current()):tbk::__impl::__Log<Ts...>(STYLE_STATUS,s,ts...,lo){}
};
template<typename... Ts>
struct info: tbk::__impl::__Log<Ts...>{
    info(std::string_view s,const Ts&... ts,const std::source_location& lo=std::source_location::current()):tbk::__impl::__Log<Ts...>(STYLE_INFO,s,ts...,lo){}
};
template<typename... Ts>
struct warning: tbk::__impl::__Log<Ts...>{
    warning(std::string_view s,const Ts&... ts,const std::source_location& lo=std::source_location::current()):tbk::__impl::__Log<Ts...>(STYLE_WARNING,s,ts...,lo){}
};
template<typename... Ts>
struct error: tbk::__impl::__Log<Ts...>{
    error(std::string_view s,const Ts&... ts,const std::source_location& lo=std::source_location::current()):tbk::__impl::__Log<Ts...>(STYLE_ERROR,s,ts...,lo){}
};
template<typename... Ts> log(fmt::text_style style,std::string_view,const Ts&...) -> log<Ts...>;
template<typename... Ts> log(std::string_view,const Ts&...) -> log<Ts...>;
template<typename... Ts> status(std::string_view,const Ts&...) -> status<Ts...>;
template<typename... Ts> info(std::string_view,const Ts&...) -> info<Ts...>;
template<typename... Ts> warning(std::string_view,const Ts&...) -> warning<Ts...>;
template<typename... Ts> error(std::string_view,const Ts&...) -> error<Ts...>;
} // namespace tbk
#endif // __TBK_LOG_H__
