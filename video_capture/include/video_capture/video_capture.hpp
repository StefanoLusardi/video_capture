#pragma once

#include "api.hpp"
#include <string>
#include <functional>
#include <memory>
#include <optional>

struct AVFormatContext;
struct AVCodecContext; 
struct AVCodec;
struct AVPacket;
struct AVFrame;
struct AVDictionary;
struct SwsContext;
struct AVBufferRef;

#define VIDEO_CAPTURE_LOG_ENABLED 1

namespace vc
{
enum class decode_support { none, SW, HW };
enum class log_level { all, info, error };

class API_VIDEO_CAPTURE video_capture
{
    class hw_acceleration;
    class logger;

public:
    explicit video_capture();
    ~video_capture();
    
    using log_callback_t = std::function<void(const std::string&)>;
    void set_log_callback(const log_callback_t& cb, const log_level& level = log_level::all);    
    bool open(const std::string& video_path, decode_support decode_preference = decode_support::none);
    auto get_frame_size() const -> std::optional<std::tuple<int, int>>;
    auto get_fps() const -> std::optional<double>;
    bool next(uint8_t** data);
    void release();

protected:
    bool grab();
    bool retrieve(uint8_t** data);
    bool decode();
    void reset();

private:
    bool _is_initialized;
    decode_support _decode_support;

    AVFormatContext* _format_ctx;
    AVCodecContext* _codec_ctx; 
    AVPacket* _packet;
    
    AVFrame* _src_frame;
    AVFrame* _dst_frame;
    AVFrame* _tmp_frame;
    
    SwsContext* _sws_ctx;
    AVDictionary* _options;
    int _stream_index;

    std::shared_ptr<logger> _logger;
    std::unique_ptr<hw_acceleration> _hw;
};

}
