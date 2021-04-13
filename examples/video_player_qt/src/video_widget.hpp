#pragma once

#define VIDEO_CAPTURE_LOG_ENABLED 1
#include <video_capture/video_capture.hpp>

#include <memory>
#include <thread>
#include <mutex>

#include <QWidget>
#include <QImage>

namespace qvp
{
class video_widget : public QWidget
{
    Q_OBJECT
    enum class state {init, open, play, stop};

public:
    explicit video_widget(QWidget* parent = nullptr);
    bool open(const std::string &video_path, bool use_hw_accel = false);
    bool play();
    bool stop();
    bool is_playing() const;
    bool is_opened() const;
    void set_smooth(bool smooth);

signals:
    void refresh();
    void started();
    void stopped();

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    std::unique_ptr<vc::video_capture> _video_capture;
    QImage _frame;
    std::thread _video_thread;
    state _state;
    Qt::TransformationMode _transformation_mode;
    void release();
};

}
