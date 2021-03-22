#include <video_capture.hpp>
#include "hw_acceleration.hpp"
#include "logger.hpp"

#include <thread>
#include <chrono>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <inttypes.h>
}

namespace vc
{
video_capture::video_capture() 
    : _logger{std::make_shared<vc::logger>()} 
    , _hw{std::make_unique<vc::hw_acceleration>(_logger)}
{
    reset(); 
    av_log_set_level(0);
}

video_capture::~video_capture()
{
    release();
}

void video_capture::set_log_callback(const log_callback_t& cb) { _logger->set_log_callback(log_level::all, cb); }

void video_capture::set_info_callback(const log_callback_t& cb) { _logger->set_log_callback(log_level::info, cb); }

void video_capture::set_error_callback(const log_callback_t& cb) { _logger->set_log_callback(log_level::error, cb); }

bool video_capture::open(const char* filename, decode_support decode_preference)
{
    log_info(_logger, "AAA", "BBB");

    if(decode_preference == decode_support::HW)
        _decode_support = _hw->init();
    else
        _decode_support = decode_support::SW;  

    if (_format_ctx = avformat_alloc_context(); !_format_ctx)
    {
        _logger->log(log_level::error, "avformat_alloc_context");
        return false;
    }

    if (auto r = av_dict_set(&_options, "rtsp_transport", "tcp", 0); r < 0)
    {
        _logger->log(log_level::error, "av_dict_set", _logger->err2str(r));
        return false;
    }

    if (auto r = avformat_open_input(&_format_ctx, filename, nullptr, &_options); r < 0)
    {
        _logger->log(log_level::error, "avformat_open_input", _logger->err2str(r));
        return false;
    }

    if (auto r = avformat_find_stream_info(_format_ctx, nullptr); r < 0)
    {
        _logger->log(log_level::error, "avformat_find_stream_info");
        return false;
    }

    AVCodec* codec = nullptr;
    if (_stream_index = av_find_best_stream(_format_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0); _stream_index < 0)
    {
        _logger->log(log_level::error, "av_find_best_stream", _logger->err2str(_stream_index));
        return false;
    }

    if (_codec_ctx = avcodec_alloc_context3(codec); !_codec_ctx)
    {
        _logger->log(log_level::error, "avcodec_alloc_context3");
        return false;
    }

    if (auto r = avcodec_parameters_to_context(_codec_ctx, _format_ctx->streams[_stream_index]->codecpar); r < 0)
    {
        _logger->log(log_level::error, "avcodec_parameters_to_context", _logger->err2str(r));
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
        _logger->log(log_level::error, "avcodec_open2", _logger->err2str(r));
        return false;
    }

    if (_packet = av_packet_alloc(); !_packet)
    {
        _logger->log(log_level::error, "av_packet_alloc");
        return false;
    }

    if (_src_frame = av_frame_alloc(); !_src_frame)
    {
        _logger->log(log_level::error, "av_frame_alloc");
        return false;
    }

    if (_dst_frame = av_frame_alloc(); !_dst_frame)
    {
        _logger->log(log_level::error, "av_frame_alloc");
        return false;
    }

    _dst_frame->format = AV_PIX_FMT_BGR24;
	_dst_frame->width  = _codec_ctx->width;
	_dst_frame->height = _codec_ctx->height;
    if (auto r = av_frame_get_buffer(_dst_frame, 0); r < 0)
    {
        _logger->log(log_level::error, "av_frame_get_buffer", _logger->err2str(r));
        return false;
    }

    if(_decode_support == decode_support::HW)
    {
        if (_hw->hw_frame = av_frame_alloc(); !_hw->hw_frame)
        {
            _logger->log(log_level::error, "av_frame_alloc");
            return false;
        }
    }

    return true;
}

auto video_capture::get_frame_size() const -> std::tuple<int, int>
{
    if(_codec_ctx != nullptr)
        return { _codec_ctx->width, _codec_ctx->height };
    
    return {-1, -1};
}

bool video_capture::grab()
{
    while(av_read_frame(_format_ctx, _packet) >= 0)
    {
        if (_packet->stream_index != _stream_index)
        {
            av_packet_unref(_packet);
            continue;
        }

        if (auto r = avcodec_send_packet(_codec_ctx, _packet); r < 0)
        {
            _logger->log(log_level::error, "avcodec_send_packet", _logger->err2str(r));
            return false;
        }

        if (auto r = avcodec_receive_frame(_codec_ctx, _src_frame); r < 0)
        {
            if (AVERROR_EOF == r || AVERROR(EAGAIN) == r)
            {
                av_packet_unref(_packet);
                continue;
            }
            else
            {
                _logger->log(log_level::error, "avcodec_receive_frame", _logger->err2str(r));
                return false;
            }
        }
        
        av_packet_unref(_packet);
        break;
    }
    
    return true;
}

/*
bool video_capture::grab()
{
    av_packet_unref(_packet);

    if(auto r = av_read_frame(_format_ctx, _packet); r < 0)
    {
        if (AVERROR_EOF == r || AVERROR(EAGAIN) == r)
            return false;

        _logger->log(log_level::error, "av_read_frame", _logger->err2str(r));
        return false;
    }

    if (_packet->stream_index != _stream_index)
        return false;

    if (auto r = avcodec_send_packet(_codec_ctx, _packet); r < 0)
    {
        _logger->log(log_level::error, "avcodec_send_packet", _logger->err2str(r));
        return false;
    }

    if (auto r = avcodec_receive_frame(_codec_ctx, _src_frame); r < 0)
    {
        _logger->log(log_level::error, "avcodec_receive_frame", _logger->err2str(r));
        return false;
    }

    // if(av_read_frame(_format_ctx, _packet) < 0)    
    //     return false;
    // avcodec_send_packet(_codec_ctx, _packet);
    // avcodec_receive_frame(_codec_ctx, _src_frame);

    return true;
}
*/

bool video_capture::retrieve(uint8_t** data)
{
    // auto pts = _src_frame->pts;

    int w = _codec_ctx->width;
    int h = _codec_ctx->height;
    
    _sws_ctx = sws_getCachedContext(_sws_ctx,
        w, h, (AVPixelFormat)_tmp_frame->format,
        w, h, AVPixelFormat::AV_PIX_FMT_BGR24,
        SWS_BICUBIC, nullptr, nullptr, nullptr);
    
    if (!_sws_ctx)
    {
        _logger->log(log_level::error, "Couldn't initialize SwsContext");
        return false;
    }

    int dest_slice_h = sws_scale(_sws_ctx,
        _tmp_frame->data, _tmp_frame->linesize, 0, h,
        _dst_frame->data, _dst_frame->linesize);

    if (dest_slice_h !=h)
    {
        _logger->log(log_level::error, "sws_scale() worked out unexpectedly");
        return false;
    }

    *data = _dst_frame->data[0];
    return true;
}

bool video_capture::decode()
{
    if (_src_frame->format == _hw->hw_pixel_format)
    {
        if (auto r = av_hwframe_transfer_data(_hw->hw_frame, _src_frame, 0); r < 0)
        {
            _logger->log(log_level::error, "av_hwframe_transfer_data", _logger->err2str(r));
            return false;
        }
        _tmp_frame = _hw->hw_frame;
    }
    else
    {
        _tmp_frame = _src_frame;
    }

    return true;

/**
 * 
 * COPY DATA BUFFER
 * 
    int frame_size = av_image_get_buffer_size((AVPixelFormat)_dst_frame->format, _dst_frame->width, _dst_frame->height, 1);
    if (frame_size < 0)
    {
        _logger->log(log_level::error, "av_image_get_buffer_size " << _logger->err2str(frame_size) << std::endl;
        return false;
    }

    if (auto r = av_image_copy_to_buffer(*data, frame_size, (const uint8_t * const *)_dst_frame->data, (const int *)_dst_frame->linesize, (AVPixelFormat)_dst_frame->format, _dst_frame->width, _dst_frame->height, 1); r < 0)
    {
        _logger->log(log_level::error, "av_image_copy_to_buffer", _logger->err2str(r));
        return false;
    }
*/
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
    _decode_support = decode_support::default;
    
    _format_ctx = nullptr;
    _codec_ctx = nullptr; 
    _packet = nullptr;

    _src_frame = nullptr;
    _dst_frame = nullptr;
    _tmp_frame = nullptr;

    _sws_ctx = nullptr;
    _options = nullptr;
    _stream_index = -1;
    _frame_pts = -1;    
}

}
