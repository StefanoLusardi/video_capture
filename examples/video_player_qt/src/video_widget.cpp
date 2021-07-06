#include "video_widget.hpp"

#include <QPainter>
#include <QDebug>

namespace qvp
{

video_widget::video_widget(QWidget* parent)
    : QWidget{parent}
    , _video_capture{std::make_unique<vc::video_capture>()}
    , _state{state::init}
{
    connect(this, &video_widget::refresh, this, [this](){ repaint(); });
}

void video_widget::paintEvent(QPaintEvent *e)
{
    (void)e;

    if (_current_frame.isNull())
        return;

    // if(!_frames.size())
    //     return;

    QPainter p(this);
    p.drawImage(0, 0, _current_frame.scaled(rect().size(), Qt::KeepAspectRatio, _transformation_mode));
}

bool video_widget::open(const std::string& video_path, bool use_hw_accel)
{
    if(_state != state::init)
      return false;

    if(_video_thread.joinable())
        _video_thread.join();
        
    auto decode_support = use_hw_accel ? vc::decode_support::HW : vc::decode_support::SW;

    if(!_video_capture->open(video_path, decode_support))
    {
      qDebug() << "Unable to open " << video_path.c_str();
      return false;
    }

    _state = state::open;
    return true;
}

bool video_widget::play()
{
    if(_state != state::open)
        return false;

    auto size = _video_capture->get_frame_size(); 
    if(!size)
    {
        qDebug() << "Unable to retrieve frame size from video";
        return false;
    }    

    auto fps = _video_capture->get_fps(); 
    if(!fps)
    {            
        qDebug() <<  "Unable to retrieve FPS from video";        
        // In case of a video stream FPS is not available, so do not return.
        // Default FPS to 1
        fps = 1;
    }

    // _frames.clear();

    _video_thread = std::thread([this, size=size.value(), fps=fps.value()]()
    {
        using namespace std::chrono_literals;
        qDebug() << "Video Started";
        
        uint8_t* frame_data = {};
        const auto [w, h] = size;
        const auto bytesPerLine = w * 3;

        emit started();
        
        const auto frame_time = std::chrono::nanoseconds(static_cast<int>(1'000'000'000/fps));

        while(true)
        {
            if(_state != state::play || !_video_capture->next(&frame_data))
                break;
            
            _current_frame = std::move(QImage(frame_data, w, h, bytesPerLine, QImage::Format_BGR888));
            emit refresh();
        }

        _current_frame = QImage();
        emit stopped();

        // const auto sleep_time = std::chrono::nanoseconds(static_cast<int>(1'000'000'000/fps));
        // auto start_time = std::chrono::steady_clock::now();
        // while(true)
        // {
        //     if(_state != state::play || !_video_capture->next(&frame_data))
        //         break;            
        //     _frames.put(std::move(QImage(frame_data, w, h, bytesPerLine, QImage::Format_BGR888)));
        //     auto elapsed_time = std::chrono::steady_clock::now() - start_time;
        //     if(elapsed_time < std::chrono::duration(sleep_time))
        //     {
        //         auto wait_time = sleep_time - elapsed_time;
        //         std::this_thread::sleep_for(wait_time);
        //     }
        //     emit refresh();
        //     start_time = std::chrono::steady_clock::now();
        // }
        //stop();
        //release();
    });

    _state = state::play;
    // _player_timer.setInterval(static_cast<int>(1000));
    // _player_timer.setInterval(static_cast<int>(1000/fps.value()));
    // _player_timer.start();
    
    return true;
}

bool video_widget::stop()
{   
    if(_state != state::play)
        return false;

    _state = state::stop;
    // _player_timer.stop();
    return true;
}

void video_widget::release()
{
    _video_capture->release();
    _state = state::init;
}

void video_widget::set_smooth(bool smooth)
{
    _transformation_mode = smooth ? Qt::SmoothTransformation : Qt::FastTransformation;
}

bool video_widget::is_playing() const
{
    return _state == state::play;
}

bool video_widget::is_opened() const
{
    return _state == state::open;
}

}