#include <video_capture/video_capture.hpp>
#include <video_capture/raw_frame.hpp>

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

namespace vc
{
std::map<log_level, logger::log_callback_t> logger::log_callbacks = {};

video_capture::video_capture() noexcept
    : _is_opened{ false }
    , _hw{std::make_unique<hw_acceleration>()}
{
    init(); 
    av_log_set_level(0);
}

video_capture::~video_capture() noexcept
{
    release();
}

void video_capture::set_log_callback(const log_callback_t& cb, const log_level& level) { vc::logger::set_log_callback(cb, level); }

bool video_capture::open(const std::string& video_path, decode_support decode_preference)
{
    std::lock_guard lock(_open_mutex);
    
    release();

    log_info("Opening video path:", video_path);
    log_info("HW acceleration", (decode_preference == decode_support::HW ? "required" : "not required"));

    if(decode_preference == decode_support::HW)
        _decode_support = _hw->init();
    else
        _decode_support = decode_support::SW;

    if (_format_ctx = avformat_alloc_context(); !_format_ctx)
    {
        log_error("avformat_alloc_context");
        return false;
    }

    if (auto r = av_dict_set(&_options, "rtsp_transport", "tcp", 0); r < 0)
    {
        log_error("av_dict_set", vc::logger::err2str(r));
        return false;
    }

    if (auto r = avformat_open_input(&_format_ctx, video_path.c_str(), nullptr, &_options); r < 0)
    {
        log_error("avformat_open_input", vc::logger::err2str(r));
        return false;
    }

    if (auto r = avformat_find_stream_info(_format_ctx, nullptr); r < 0)
    {
        log_error("avformat_find_stream_info");
        return false;
    }

    AVCodec* codec = nullptr;
    if (_stream_index = av_find_best_stream(_format_ctx, AVMediaType::AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0); _stream_index < 0)
    {
        log_error("av_find_best_stream", vc::logger::err2str(_stream_index));
        return false;
    }

    if (_codec_ctx = avcodec_alloc_context3(codec); !_codec_ctx)
    {
        log_error("avcodec_alloc_context3");
        return false;
    }

    if (auto r = avcodec_parameters_to_context(_codec_ctx, _format_ctx->streams[_stream_index]->codecpar); r < 0)
    {
        log_error("avcodec_parameters_to_context", vc::logger::err2str(r));
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
        log_error("avcodec_open2", vc::logger::err2str(r));
        return false;
    }

    if (_packet = av_packet_alloc(); !_packet)
    {
        log_error("av_packet_alloc");
        return false;
    }
    
    if (_src_frame = av_frame_alloc(); !_src_frame)
    {
        log_error("av_frame_alloc");
        return false;
    }

    if (_dst_frame = av_frame_alloc(); !_dst_frame)
    {
        log_error("av_frame_alloc");
        return false;
    }

    _tmp_frame = _src_frame;

    _dst_frame->format = AVPixelFormat::AV_PIX_FMT_BGR24;
	_dst_frame->width  = _codec_ctx->width;
	_dst_frame->height = _codec_ctx->height;
    if (auto r = av_frame_get_buffer(_dst_frame, 32); r < 0)
    {
        log_error("av_frame_get_buffer", vc::logger::err2str(r));
        return false;
    }

    _is_opened = true;
    log_info("Opened video path:", video_path);
    log_info("Frame Width:", _codec_ctx->width, "px");
    log_info("Frame Height:", _codec_ctx->height, "px");
    log_info("Frame Rate:", (get_fps() != std::nullopt ? get_fps().value() : -1), "fps");
    log_info("Duration:", (get_duration() != std::nullopt ? std::chrono::duration_cast<std::chrono::seconds>(get_duration().value()).count() : -1), "sec");
    log_info("Number of frames:", (get_frame_count() != std::nullopt ? get_frame_count().value() : -1));
    log_info("Video Capture is initialized");

    return true;
}

bool video_capture::is_opened() const
{
    return _is_opened;
}

auto video_capture::get_frame_count() const -> std::optional<int>
{
    if(!_is_opened)
    {
        log_error("Frame count not available. Video path must be opened first.");
        return std::nullopt;
    }

    auto nb_frames = _format_ctx->streams[_stream_index]->nb_frames;
    if (!nb_frames)
    {
        double duration_sec = static_cast<double>(_format_ctx->duration) / static_cast<double>(AV_TIME_BASE);
        auto fps = get_fps();
        nb_frames = std::floor(duration_sec * fps.value() + 0.5);
    }
    if (nb_frames)
        return std::make_optional(static_cast<int>(nb_frames));
    
    return std::nullopt;
}

auto video_capture::get_duration() const -> std::optional<std::chrono::steady_clock::duration>
{
    if(!_is_opened)
    {
        log_error("Duration not available. Video path must be opened first.");
        return std::nullopt;
    }

    auto duration = std::chrono::duration<int64_t, std::ratio<1, AV_TIME_BASE>>(_format_ctx->duration);
    return std::make_optional(duration);
}

auto video_capture::get_frame_size() const -> std::optional<std::tuple<int, int>>
{
    if(!_is_opened)
    {
        log_error("Frame size not available. Video path must be opened first.");
        return std::nullopt;
    }
    
    auto size = std::make_tuple(_codec_ctx->width, _codec_ctx->height);
    return std::make_optional(size);
}

auto video_capture::get_frame_size_in_bytes() const -> std::optional<int>
{
    if(!_is_opened)
    {
        log_error("Frame size in bytes not available. Video path must be opened first.");
        return std::nullopt;
    }

    auto bytes = _codec_ctx->width * _codec_ctx->height * 3;
    return std::make_optional(bytes);
}

auto video_capture::get_fps() const -> std::optional<double>
{
    if(!_is_opened)
    {
        log_error("FPS not available. Video path must be opened first.");
        return std::nullopt;
    }
    
    auto frame_rate = _format_ctx->streams[_stream_index]->avg_frame_rate;
    if(frame_rate.num <= 0 || frame_rate.den <= 0 )
    {
        log_info("Unable to retrieve FPS.");
        return std::nullopt;
    }

    auto fps = static_cast<double>(frame_rate.num) / static_cast<double>(frame_rate.den);
    return std::make_optional(fps);
}

bool video_capture::is_error(const char* func_name, const int error) const
{
    if(AVERROR_EOF == error) 
    {
        log_info(func_name, vc::logger::err2str(error));
        return false;
    }
    
    log_error(func_name, vc::logger::err2str(error));  
    return true;    
}

bool video_capture::grab()
{
    while(true)
    {
        av_packet_unref(_packet);
        if(auto r = av_read_frame(_format_ctx, _packet); r < 0)
        {
            if (AVERROR(EAGAIN) == r)
                continue; 

            if(is_error("av_read_frame", r))
                return false;
        }

        if (_packet->stream_index != _stream_index)
            continue;

        if (auto r = avcodec_send_packet(_codec_ctx, _packet); r < 0)
        {
            if (AVERROR(EAGAIN) == r)                         
                continue; 
            
            if(is_error("avcodec_send_packet", r))
                return false;
        }

        if (auto r = avcodec_receive_frame(_codec_ctx, _src_frame); r < 0)
        {
            if (AVERROR(EAGAIN) == r)                         
                continue; 
            
            log_info("avcodec_receive_frame", vc::logger::err2str(r));
            // release();
            return false;
        }
        
        return true;
    }
}

bool video_capture::decode()
{
    if (_src_frame->format == _hw->hw_pixel_format)
    {
        if (auto r = av_hwframe_transfer_data(_tmp_frame, _src_frame, 0); r < 0)
        {
            log_error("av_hwframe_transfer_data", vc::logger::err2str(r));
            return false;
        }

        if (auto r = av_frame_copy_props(_tmp_frame, _src_frame); r < 0)
        {
            log_error("av_frame_copy_props", vc::logger::err2str(r));
            return false;
        }
    }
    else
    {
        _tmp_frame = _src_frame;
    }

    return true;
}

bool video_capture::retrieve()
{
    if (!_sws_ctx)
    {
        _sws_ctx = sws_getCachedContext(_sws_ctx,
            _codec_ctx->width, _codec_ctx->height, (AVPixelFormat)_tmp_frame->format,
            _codec_ctx->width, _codec_ctx->height, AVPixelFormat::AV_PIX_FMT_BGR24,
            SWS_BICUBIC, nullptr, nullptr, nullptr);
        
        if (!_sws_ctx)
        {
            log_error("Unable to initialize SwsContext");
            return false;
        }
    }

    _dst_frame->linesize[0] = _codec_ctx->width * 3;
    sws_scale(_sws_ctx, _tmp_frame->data, _tmp_frame->linesize,
        0, _codec_ctx->height, _dst_frame->data, _dst_frame->linesize);

    return true;
}

bool video_capture::read(uint8_t** data)
{
    if(!grab())
        return false;

    if(!decode())
        return false;

    if(!retrieve())
        return false;

    *data = _dst_frame->data[0];
    return true;
}

bool video_capture::read(raw_frame* frame)
{
    if(!grab())
        return false;

    if(!decode())
        return false;

    _dst_frame->data[0] = frame->data.data();
    if(!retrieve())
        return false;

    const auto time_base = _format_ctx->streams[_stream_index]->time_base;
    frame->pts = _tmp_frame->best_effort_timestamp * static_cast<double>(time_base.num) / static_cast<double>(time_base.den);
    
    return true;
}

void video_capture::release()
{
    if(!_is_opened)
        return;

    log_info("Release video capture");

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

    init();
    _hw->release();
}

void video_capture::init()
{
    log_info("Reset video capture");

    _is_opened = false;
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
