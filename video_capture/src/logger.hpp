#pragma once

#include <sstream>
#include <utility>
#include <functional>
#include <cstring>
#include <map>

extern "C"
{
#include <libavutil/error.h>
}

#if defined(VIDEO_CAPTURE_LOG_ENABLED)
    #define log_info(...) vc::logger::log(log_level::info, ##__VA_ARGS__)
    #define log_error(...) vc::logger::log(log_level::error, ##__VA_ARGS__)
#else
    #define log_info(...) (void)0
    #define log_error(...) (void)0
#endif

namespace vc
{
class logger
{
public:
    template<typename... Args>
    static void log(const log_level& level, Args&& ...args)
    {
        if(auto cb = log_callbacks.find(level); cb != log_callbacks.end())
        {
            std::stringstream s;
            ((s << std::forward<Args>(args) << ' ') , ...);
            cb->second(s.str());
        }
    }

    using log_callback_t = std::function<void(const std::string&)>;
    static void set_log_callback(const log_callback_t& cb, const log_level& level)
    {
        if(level == log_level::all)
        {
            log_callbacks[log_level::info] = cb;
            log_callbacks[log_level::error] = cb;
            return;
        }

        log_callbacks[level] = cb;
    }

    static const char* err2str(int errnum)
    {
        static char str[AV_ERROR_MAX_STRING_SIZE];
        std::memset(str, 0, sizeof(str));
        return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
    }

private:
    static std::map<log_level, log_callback_t> log_callbacks;
};

}
