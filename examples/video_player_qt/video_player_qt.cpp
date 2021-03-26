#include <QString>
#include <QDebug>
#include <QMainWindow>
#include <QApplication>

class MainWindow : public QMainWindow
{
    // Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {}
    ~MainWindow() {}
};

int main(int argc, char** argv)
{
    qDebug() << QString("qString");
    QApplication app(argc, argv);
    MainWindow mw;
    mw.show();
    return app.exec();
}