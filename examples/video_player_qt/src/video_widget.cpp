#include "video_widget.hpp"
#include <QPainter>
#include <QDebug>
#include <iostream>

namespace qvp
{

video_widget::video_widget(QWidget* parent)
    : QWidget{parent}
    , _video_capture{std::make_unique<vc::video_capture>()}
    , _state{state::init}
{
    connect(this, &video_widget::refresh, this, [this](){repaint();});
}

void video_widget::paintEvent(QPaintEvent *e)
{
    (void)e;

    if(_frame.isNull())
        return;

    QPainter p(this);
    p.drawImage(0, 0, _frame.scaled(rect().size(), Qt::KeepAspectRatio));
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
      std::cout << "Unable to open " << video_path << std::endl;      
      return false;
    }

    _state = state::open;
    return true;
}

bool video_widget::play()
{
    {
        std::scoped_lock lock(_state_mutex);        
        if(_state != state::open)
            return false;
    }

    auto size = _video_capture->get_frame_size(); 
    if(!size)
    {
        qDebug() <<  "Unable to retrieve frame size from video";
        return false;
    }    

    auto fps = _video_capture->get_fps(); 
    if(!fps)
    {            
        qDebug() <<  "Unable to retrieve FPS from video";
        
        // In case of a video stream FPS is not available, so do not return.
        //return false;

        // Default FPS to 1
        fps = 1;
    }

    _video_thread = std::thread([this, size=size.value(), fps=fps.value()]()
    {
        using namespace std::chrono_literals;
        qDebug() << "Video Started";
        
        uint8_t* frame_data = {};
        const auto [w, h] = size;
        const auto bytesPerLine = w * 3;
        const auto sleep_time = std::chrono::milliseconds((int)fps);
        
        while(true)
        {
            {
                std::scoped_lock lock(_state_mutex);                
                if(_state != state::play || !_video_capture->next(&frame_data))
                    break;
                
                _frame = QImage(frame_data, w, h, bytesPerLine, QImage::Format_BGR888);
            }

            emit refresh();

            std::this_thread::sleep_for(sleep_time);
        }
        stop();
    });

    _state = state::play;
    return true;
}

bool video_widget::stop()
{
    {
        std::scoped_lock lock(_state_mutex);
        if(_state != state::play)
            return false;

        _state = state::stop;
        _frame = QImage();
    }

    emit refresh();

    _video_capture->release();
    emit stopped();

    {
        std::scoped_lock lock(_state_mutex); 
        _state = state::init;
    }

    return true;
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