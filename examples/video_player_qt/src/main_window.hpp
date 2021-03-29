#pragma once

#include <QMainWindow>

namespace Ui {
class main_window;
}

namespace qvp
{
class main_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit main_window(QWidget *parent = nullptr);
    ~main_window();

private:
    Ui::main_window *_ui;
};

}
