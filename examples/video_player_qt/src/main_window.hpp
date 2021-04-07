#pragma once

#include <QMainWindow>

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

private slots:
    void on_video_stopped();
    void on_video_path_changed(const QString& str);
    
private:
    void toggle_player_state();
    void open_video(const QString& file_name);

    QIcon _play_icon;
    QIcon _stop_icon;
    Ui::main_window* _ui;
};

}
