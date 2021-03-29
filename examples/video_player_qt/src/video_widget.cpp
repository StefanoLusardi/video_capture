#include "video_widget.hpp"
#include <QPainter>
#include <QDebug>
#include <iostream>

namespace qvp
{

video_widget::video_widget(QWidget* parent)
    : QWidget(parent)
    , _video_capture{std::make_unique<vc::video_capture>()}
    , _state{state::init}
    , _frame{QImage()}
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
    if(_state != state::open)
      return false;

    _video_thread = std::thread([this]()
    {
        using namespace std::chrono_literals;
        qDebug() << "Video Started";
        auto [w, h] = _video_capture->get_frame_size();

        uint8_t* frame_data = {};
        while(_video_capture->next(&frame_data))
        {
            _frame = QImage(frame_data, w, h, w*3,  QImage::Format_BGR888);
            emit refresh();
            std::this_thread::sleep_for(100ms);
        }
        qDebug() << "Video Finished";
        stop();
    });

    _state = state::play;
    return true;
}

bool video_widget::stop()
{
    if(_state != state::play)
        return false;
    
    _frame = QImage();
    _video_capture->release();
    emit refresh();

    _state = state::init;
    return true;
}

}