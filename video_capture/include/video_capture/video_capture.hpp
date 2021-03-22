#pragma once

#include "api.hpp"

struct AVFormatContext;
struct AVCodecContext; 
struct AVCodec;
struct AVPacket;
struct AVFrame;
struct AVDictionary;
struct SwsContext;
struct AVBufferRef;

#include <stdint.h>
#include <vector>
#include <functional>
#include <map>
#include <sstream>

/** 
 * The best thing to do in order to avoid color conversion in different toolkits
 * is to specify different AVPixelFormat to be used in video_capture::retrieve()
 * Qt:      AV_PIX_FMT_RGB24 
 * OpenCV:  AV_PIX_FMT_BGR24 
*/

// __DATE__, __FILE__, __LINE__,

#define VIDEO_CAPTURE_LOG_ENABLED 1

#if defined(VIDEO_CAPTURE_LOG_ENABLED)
    #define log_info(logger, ...) logger->log(vc::log_level::info, ##__VA_ARGS__)
    #define log_error(logger, ...) logger->log(vc::log_level::error, ##__VA_ARGS__)
#else
    #define log_info(logger, ...) (void)0
    #define log_error(logger, ...) (void)0
#endif

namespace vc
{
class hw_acceleration;
class logger;

enum class decode_support { default, SW, HW };

class API_VIDEO_CAPTURE video_capture_params
{
    // decode_support
    // hw_device_type
    // output format OpenCV/Qt [BGR/RGB]
};

class API_VIDEO_CAPTURE video_capture
{
public:
    explicit video_capture();
    ~video_capture();
    
    using log_callback_t = std::function<void(const std::string&)>;
    void set_log_callback(const log_callback_t& cb);
    void set_info_callback(const log_callback_t& cb);
    void set_error_callback(const log_callback_t& cb);
    
    bool open(const char* filename, decode_support decode_preference = decode_support::default);
    auto get_frame_size() const -> std::tuple<int, int>;
    bool next(uint8_t** data);
    void release();

protected:
    bool grab();
    bool retrieve(uint8_t** data);
    bool decode();
    void reset();

private:
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
    int /*int64_t*/ _frame_pts;

    std::shared_ptr<vc::logger> _logger;
    std::unique_ptr<vc::hw_acceleration> _hw;
};

}
