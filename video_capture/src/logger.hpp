#pragma once

extern "C"
{
#include <libavutil/error.h>
}

#include <sstream>
#include <utility>
#include <functional>
#include <cstring>
#include <map>

#if defined(VIDEO_CAPTURE_LOG_ENABLED)
    #define log_info(logger, ...) logger->log(log_level::info, __DATE__, __FILE__, __LINE__, ##__VA_ARGS__)
    #define log_error(logger, ...) logger->log(log_level::error, __DATE__, __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define log_info(logger, ...) (void)0
    #define log_error(logger, ...) (void)0
#endif

namespace vc
{
class video_capture::logger
{
public:
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

    const char* err2str(int errnum) const
    {
        static char str[AV_ERROR_MAX_STRING_SIZE];
        std::memset(str, 0, sizeof(str));
        return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
    }

private:
    std::map<log_level, log_callback_t> log_callbacks;
};

}
