#pragma once

extern "C"
{
#include <libavutil/error.h>
}

namespace vc
{
enum class log_level { all, info, error };

class logger
{
public:
    template<typename... Args>
    void log(vc::log_level level, Args&& ...args)
    {
        if(auto cb = log_callbacks.find(level); cb != log_callbacks.end())
        {
            std::stringstream s;
            ((s << std::forward<Args>(args) << ' ') , ...);
            cb->second(s.str());
        }
    }

    using log_callback_t = std::function<void(const std::string&)>;
    void set_log_callback(vc::log_level level, const log_callback_t& cb)
    {
        if(level == log_level::all)
        {
            log_callbacks[log_level::info] = cb;
            log_callbacks[log_level::error] = cb;
            return;
        }

        log_callbacks[level] = cb;
    }

    // const char* err2str(int errnum) const;
    const char* err2str(int errnum) const
    {
        static char str[AV_ERROR_MAX_STRING_SIZE];
        memset(str, 0, sizeof(str));
        return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
    }

private:
    std::map<vc::log_level, log_callback_t> log_callbacks;
};

}
