#pragma once

#define VIDEO_CAPTURE_LOG_ENABLED 1
#include <video_capture/video_capture.hpp>

#include <memory>
#include <thread>

#include <QWidget>
#include <QImage>

namespace qvp
{
class video_widget : public QWidget
{
    Q_OBJECT
    enum class state {init, open, play};

public:
    explicit video_widget(QWidget* parent = nullptr);
    bool open(const std::string &video_path, bool use_hw_accel = false);
    bool play();
    bool stop();

signals:
    void refresh();

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    std::unique_ptr<vc::video_capture> _video_capture;
    std::thread _video_thread;
    state _state;

    QImage _frame;
};

}