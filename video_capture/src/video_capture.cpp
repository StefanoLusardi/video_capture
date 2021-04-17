#include <video_capture.hpp>
#include "logger.hpp"
#include "hw_acceleration.hpp"

#include <thread>
#include <chrono>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>
#include <libavutil/buffer.h>
#include <libavutil/hwcontext.h>
}

#define CHECK_ERROR(r, func) {                          \
    if (AVERROR(EAGAIN) == r)                           \
        continue;                                       \
    if(AVERROR_EOF == r) {                              \
        log_info(_logger, func, _logger->err2str(r));   \
        return false;                                   \
    }                                                   \
    log_error(_logger, func, _logger->err2str(r));      \
    return false;                                       \
}

namespace vc
{
video_capture::video_capture() 
    : _is_initialized{ false }
    , _logger{std::make_shared<logger>()} 
    , _hw{std::make_unique<hw_acceleration>(_logger)}
{
    reset(); 
    av_log_set_level(0);
}

video_capture::~video_capture()
{
    release();
}

void video_capture::set_log_callback(const log_callback_t& cb, const log_level& level) { _logger->set_log_callback(cb, level); }

bool video_capture::open(const std::string& video_path, decode_support decode_preference)
{
    log_info(_logger, "Opening video path:", video_path);
    log_info(_logger, "HW acceleration", (decode_preference == decode_support::HW ? "required" : "not required"));

    if(decode_preference == decode_support::HW)
        _decode_support = _hw->init();
    else
        _decode_support = decode_support::SW;

    if (_format_ctx = avformat_alloc_context(); !_format_ctx)
    {
        log_error(_logger, "avformat_alloc_context");
        return false;
    }

    if (auto r = av_dict_set(&_options, "rtsp_transport", "tcp", 0); r < 0)
    {
        log_error(_logger, "av_dict_set", _logger->err2str(r));
        return false;
    }

    if (auto r = avformat_open_input(&_format_ctx, video_path.c_str(), nullptr, &_options); r < 0)
    {
        log_error(_logger, "avformat_open_input", _logger->err2str(r));
        return false;
    }

    if (auto r = avformat_find_stream_info(_format_ctx, nullptr); r < 0)
    {
        log_error(_logger, "avformat_find_stream_info");
        return false;
    }

    AVCodec* codec = nullptr;
    if (_stream_index = av_find_best_stream(_format_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0); _stream_index < 0)
    {
        log_error(_logger, "av_find_best_stream", _logger->err2str(_stream_index));
        return false;
    }

    if (_codec_ctx = avcodec_alloc_context3(codec); !_codec_ctx)
    {
        log_error(_logger, "avcodec_alloc_context3");
        return false;
    }

    if (auto r = avcodec_parameters_to_context(_codec_ctx, _format_ctx->streams[_stream_index]->codecpar); r < 0)
    {
        log_error(_logger, "avcodec_parameters_to_context", _logger->err2str(r));
        return false;
    }

    if(_decode_support == decode_support::HW)
    {
        // _codec_ctx->sw_pix_fmt = AV_PIX_FMT_NV12;
        _codec_ctx->hw_device_ctx = av_buffer_ref(_hw->hw_device_ctx);
        // _codec_ctx->hw_frames_ctx = _hw->get_frames_ctx(_codec_ctx->width, _codec_ctx->height);
    }

    if (auto r = avcodec_open2(_codec_ctx, codec, nullptr); r < 0)
    {
        log_error(_logger, "avcodec_open2", _logger->err2str(r));
        return false;
    }

    if (_packet = av_packet_alloc(); !_packet)
    {
        log_error(_logger, "av_packet_alloc");
        return false;
    }
    
    if (_tmp_frame = av_frame_alloc(); !_tmp_frame)
    {
        log_error(_logger, "av_frame_alloc");
        return false;
    }

    if (_src_frame = av_frame_alloc(); !_src_frame)
    {
        log_error(_logger, "av_frame_alloc");
        return false;
    }

    if (_dst_frame = av_frame_alloc(); !_dst_frame)
    {
        log_error(_logger, "av_frame_alloc");
        return false;
    }

    _dst_frame->format = AV_PIX_FMT_BGR24;
	_dst_frame->width  = _codec_ctx->width;
	_dst_frame->height = _codec_ctx->height;
    if (auto r = av_frame_get_buffer(_dst_frame, 0); r < 0)
    {
        log_error(_logger, "av_frame_get_buffer", _logger->err2str(r));
        return false;
    }

    // if(_decode_support == decode_support::HW)
    // {
    //     if (_hw->hw_frame = av_frame_alloc(); !_hw->hw_frame)
    //     {
    //         log_error(_logger, "av_frame_alloc");
    //         return false;
    //     }
    // }

    auto time_base = _format_ctx->streams[_stream_index]->time_base;
    if(time_base.num <= 0 || time_base.den <= 0 )
    {
        log_info(_logger, "Unable to retrieve unit time for timestamps");
        return false;
    }
    _timestamp_unit = static_cast<double>(time_base.num) / static_cast<double>(time_base.den);

    _is_initialized = true;
    log_info(_logger, "Opened video path:", video_path);
    log_info(_logger, "Frame Width:", _format_ctx->streams[_stream_index]->codec->width);
    log_info(_logger, "Frame Height:", _format_ctx->streams[_stream_index]->codec->height);
    log_info(_logger, "Frame Rate:", (get_fps() != std::nullopt ? get_fps().value() : -1));
    log_info(_logger, "Duration:", _format_ctx->streams[_stream_index]->duration);
    log_info(_logger, "Number of frames:", _format_ctx->streams[_stream_index]->nb_frames);

    // auto dd = get_duration();
    // auto ss = std::chrono::duration_cast<std::chrono::seconds>(dd.value());

    return true;
}

// auto video_capture::get_duration() const -> std::optional<std::chrono::microseconds>
// {
//     auto duration = std::chrono::duration<int64_t,std::ratio<1,AV_TIME_BASE>>(_format_ctx->duration);
//     return std::make_optional(duration);
// }

auto video_capture::get_frame_size() const -> std::optional<std::tuple<int, int>>
{
    if(!_is_initialized)
    {
        log_error(_logger, "Frame size not available. Video path must be opened first.");
        return std::nullopt;
    }
    
    auto size = std::make_tuple(_codec_ctx->width, _codec_ctx->height);
    return std::make_optional(size);
}

auto video_capture::get_fps() const -> std::optional<double>
{
    if(!_is_initialized)
    {
        log_error(_logger, "FPS not available. Video path must be opened first.");
        return std::nullopt;
    }
    
    auto frame_rate = _format_ctx->streams[_stream_index]->avg_frame_rate;
    if(frame_rate.num <= 0 || frame_rate.den <= 0 )
    {
        log_info(_logger, "Unable to retrieve FPS.");
        return std::nullopt;
    }

    auto fps = static_cast<double>(frame_rate.num) / static_cast<double>(frame_rate.den);
    return std::make_optional(fps);
}

bool video_capture::grab()
{
    while(true)
    {
        av_packet_unref(_packet);
        if(auto r = av_read_frame(_format_ctx, _packet); r < 0)
            CHECK_ERROR(r, "av_read_frame");

        if (_packet->stream_index != _stream_index)
            continue;

        if (auto r = avcodec_send_packet(_codec_ctx, _packet); r < 0)
            CHECK_ERROR(r, "avcodec_send_packet");

        if (auto r = avcodec_receive_frame(_codec_ctx, _src_frame); r < 0)
            CHECK_ERROR(r, "avcodec_receive_frame");
        
        return true;
    }
}

bool video_capture::retrieve(uint8_t** data)
{
    _sws_ctx = sws_getCachedContext(_sws_ctx,
        _codec_ctx->width, _codec_ctx->height, (AVPixelFormat)_tmp_frame->format,
        _codec_ctx->width, _codec_ctx->height, AVPixelFormat::AV_PIX_FMT_BGR24,
        SWS_BICUBIC, nullptr, nullptr, nullptr);
    
    if (!_sws_ctx)
    {
        log_error(_logger, "Unable to initialize SwsContext");
        return false;
    }

    _dst_frame->linesize[0] = _codec_ctx->width * 3;
    int dest_slice_h = sws_scale(_sws_ctx,
        _tmp_frame->data, _tmp_frame->linesize, 0, _codec_ctx->height,
        _dst_frame->data, _dst_frame->linesize);

    if (dest_slice_h != _codec_ctx->height)
    {
        log_error(_logger, "sws_scale() worked out unexpectedly");
        return false;
    }

    *data = _dst_frame->data[0];
    double sec = (_packet->dts - _format_ctx->streams[_stream_index]->start_time) * _timestamp_unit;
    auto __pts = _packet->dts * _timestamp_unit;
    auto picture_pts = _src_frame->pkt_pts != AV_NOPTS_VALUE && _src_frame->pkt_pts != 0 ? _src_frame->pkt_pts : _src_frame->pkt_dts;
    return true;
}

bool video_capture::decode()
{
    if (_src_frame->format == _hw->hw_pixel_format)
    {
        if (auto r = av_hwframe_transfer_data(_tmp_frame, _src_frame, 0); r < 0)
        // if (auto r = av_hwframe_transfer_data(_hw->hw_frame, _src_frame, 0); r < 0)
        {
            log_error(_logger, "av_hwframe_transfer_data", _logger->err2str(r));
            return false;
        }

        if (auto r = av_frame_copy_props(_tmp_frame, _src_frame); r < 0)
        {
            log_error(_logger, "av_frame_copy_props", _logger->err2str(r));
            return false;
        }
        // _tmp_frame = _hw->hw_frame;
    }
    else
    {
        _tmp_frame = _src_frame;
    }

    return true;
}

bool video_capture::next(uint8_t** data)
{
    if(!grab())
        return false;

    if(!decode())
        return false;

    return retrieve(data);
}

void video_capture::release()
{
    log_info(_logger, "Release video capture");

    if(_sws_ctx)
        sws_freeContext(_sws_ctx);

    if(_codec_ctx)
        avcodec_free_context(&_codec_ctx);

    if(_format_ctx)
    {
        avformat_close_input(&_format_ctx);
        avformat_free_context(_format_ctx);
    }

    if (_options)
       av_dict_free(&_options);

    if(_packet)
        av_packet_free(&_packet);

    if(_src_frame)
        av_frame_free(&_src_frame);

    if(_dst_frame)
        av_frame_free(&_dst_frame);

    reset();
    _hw->release();
}

void video_capture::reset()
{
    log_info(_logger, "Reset video capture");

    _is_initialized = false;
    _decode_support = decode_support::none;
    
    _format_ctx = nullptr;
    _codec_ctx = nullptr; 
    _packet = nullptr;

    _src_frame = nullptr;
    _dst_frame = nullptr;
    _tmp_frame = nullptr;

    _sws_ctx = nullptr;
    _options = nullptr;
    _stream_index = -1;
}

}
