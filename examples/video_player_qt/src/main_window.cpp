#include "main_window.hpp"
#include "ui_main_window.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QImage>
#include <QDebug>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>

namespace qvp
{
main_window::main_window(QWidget *parent)
    : QMainWindow{parent}
    , _ui{new Ui::main_window}
{
    _ui->setupUi(this);
    _ui->video_play_button->setEnabled(false);

    _play_icon.addFile(QString::fromUtf8(":/light/play"), QSize(), QIcon::Normal, QIcon::Off);
    _stop_icon.addFile(QString::fromUtf8(":/light/stop"), QSize(), QIcon::Normal, QIcon::Off);

    _player_timer.setInterval(std::chrono::seconds(1));
    connect(&_player_timer, &QTimer::timeout, this, [this](){ 
        auto v = _ui->frame_counter->intValue(); 
        _ui->frame_counter->display(++v);
    });

    setAcceptDrops(true);

    connect(_ui->video_path_search_button, &QAbstractButton::clicked, this, [this](bool){
        const auto file_name = QFileDialog::getOpenFileName(this, "Open Video", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), "Video(*.mp4 *.mkv *.mov)");
        _ui->video_path_entry->setText(file_name);
    });

    connect(_ui->video_player, &qvp::video_widget::started, this, [this](){ on_video_started(); });
    connect(_ui->video_player, &qvp::video_widget::stopped, this, [this](){ on_video_stopped(); });
    connect(_ui->video_play_button, &QAbstractButton::clicked, this, [this](bool){toggle_player_state(); });
    connect(_ui->video_path_entry, &QLineEdit::textChanged, this, [this](const QString& str){ on_video_path_changed(str); });
    connect(_ui->smooth, &QCheckBox::stateChanged, this, [this](int state){ _ui->video_player->set_smooth(state == Qt::Checked); });
}

main_window::~main_window()
{
    delete _ui;
}

void main_window::on_video_started()
{
    _player_timer.stop();
    _ui->frame_counter->display(0);
    _player_timer.start();
}

void main_window::on_video_stopped()
{
    _player_timer.stop();
    _ui->frame_counter->display(0);    
    _ui->video_play_button->setText("Play");
    _ui->video_play_button->setIcon(_play_icon);
    _ui->video_play_button->setEnabled(!_ui->video_path_entry->text().isEmpty());
    _ui->hw_accel->setEnabled(true);
    _ui->video_player->repaint();
}

void main_window::on_video_path_changed(const QString& str)
{
    if(_ui->video_player->is_playing())
        return;

    _ui->video_play_button->setEnabled(!str.isEmpty());
}

void main_window::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    QString file_path = urls.first().toString();
    if(file_path.startsWith("file:///"))
        file_path.replace("file:///", "");

    event->acceptProposedAction();
    _ui->video_path_entry->setText(file_path);
}

void main_window::dragEnterEvent(QDragEnterEvent* event)
{
    if(!event->mimeData()->hasUrls())
    {
        event->ignore();
        return;
    }

    event->accept();
}

void main_window::closeEvent(QCloseEvent *event)
{
    _ui->video_player->stop();
    event->accept();
}

void main_window::toggle_player_state()
{
    if(_ui->video_player->is_playing())
    {
        if(!_ui->video_player->stop())
            return;
    }
    else
    {
        if(!_ui->video_player->is_opened())
        {
            const auto video_path = _ui->video_path_entry->text();
           _ui->video_player->open(video_path.toStdString(), _ui->hw_accel->isChecked());
           _ui->hw_accel->setEnabled(false);
        }

        if(!_ui->video_player->play())
        {
            _ui->hw_accel->setEnabled(true);
            return;
        }

        _ui->video_play_button->setText("Stop");
        _ui->video_play_button->setIcon(_stop_icon);
    }
}

}
