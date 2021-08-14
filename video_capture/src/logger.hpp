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
    #include <mutex>
    #include <iostream>
    #define log_info(...) vc::logger::get().log(log_level::info, ##__VA_ARGS__)
    #define log_error(...) vc::logger::get().log(log_level::error, ##__VA_ARGS__)
#else
    #define log_info(...) (void)0
    #define log_error(...) (void)0
#endif

namespace vc
{
class logger
{
public:
    logger()
    {
    #if defined(VIDEO_CAPTURE_LOG_ENABLED)
        set_log_callback([this](const std::string& str){ default_callback_info(str); }, log_level::info);
        set_log_callback([this](const std::string& str){ default_callback_error(str); }, log_level::error);
    #endif
    }

    static logger& get()
    {
        static logger instance;
        return instance;
    }

    template<typename... Args>
    void log(const log_level& level, Args&& ...args)
    {
        if(auto cb = log_callbacks.find(level); cb != log_callbacks.end())
        {
            std::stringstream s;
            ((s << std::forward<Args>(args) << ' ') , ...);
            cb->second(s.str());
        }
    }

    using log_callback_t = std::function<void(const std::string&)>;
    void set_log_callback(const log_callback_t& cb, const log_level& level)
    {
        if(level == log_level::all)
        {
            log_callbacks[log_level::info] = cb;
            log_callbacks[log_level::error] = cb;
            return;
        }

        log_callbacks[level] = cb;
    }

    const char* err2str(int errnum)
    {
        static char str[AV_ERROR_MAX_STRING_SIZE];
        std::memset(str, 0, sizeof(str));
        return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
    }

#if defined(VIDEO_CAPTURE_LOG_ENABLED)
protected:
    void default_callback_info(const std::string& str) 
    {
        std::scoped_lock lock(_default_mutex);
        std::cout << "[::  INFO ::] " << str << std::endl;
    }

    void default_callback_error(const std::string& str)
    {
        std::scoped_lock lock(_default_mutex);
        std::cout << "[:: ERROR ::] " << str << std::endl;
    }

    std::mutex _default_mutex;
#endif

private:
    std::map<log_level, log_callback_t> log_callbacks;
};

}
