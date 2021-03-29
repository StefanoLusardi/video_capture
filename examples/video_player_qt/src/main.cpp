#include "main_window.hpp"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    qvp::main_window mw;
    mw.show();
    return a.exec();
}
