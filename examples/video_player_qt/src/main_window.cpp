#include "main_window.hpp"
#include "ui_main_window.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QImage>
#include <QDebug>

namespace qvp
{
main_window::main_window(QWidget *parent)
    : QMainWindow{parent}
    , _ui{new Ui::main_window}
{
    _ui->setupUi(this);
    _ui->video_play_button->setEnabled(false);

    connect(_ui->video_path_search_button, &QAbstractButton::clicked, this, [this](bool){
        const auto file_name = QFileDialog::getOpenFileName(
            this, 
            "Open Video", 
            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), 
            "Video(*.mp4 *.mkv *.mov)");

        _ui->video_path_entry->setText(file_name);
        _ui->video_play_button->setEnabled(true);
        _ui->video_player->open(file_name.toStdString());
    });

    connect(_ui->video_play_button, &QAbstractButton::clicked, this, [this](bool){
        _ui->video_play_button->setEnabled(false);
        _ui->video_player->play();
    });
}

main_window::~main_window()
{
    delete _ui;
}

}
