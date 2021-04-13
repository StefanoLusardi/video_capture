#pragma once

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class main_window;
}

class QDropEvent;
class QDragEnterEvent;

namespace qvp
{
class main_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit main_window(QWidget *parent = nullptr);
    ~main_window();

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_video_stopped();
    void on_video_started();
    void on_video_path_changed(const QString& str);
    
private:
    void toggle_player_state();
    QTimer _player_timer;
    QIcon _play_icon;
    QIcon _stop_icon;
    Ui::main_window* _ui;
};

}
