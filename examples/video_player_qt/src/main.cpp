/**
 * example: 	video_player_qt
 * author:		Stefano Lusardi
 * date:		Jun 2021
 * description:	Full application example of a very simple Qt based video player based on cv::video_capture. 
*/

#include "main_window.hpp"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("Qt Video Player");
    QApplication::setOrganizationName("qvp");
    QApplication::setOrganizationDomain("qvp.com");
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    
    qvp::main_window mw;
    mw.show();
    return a.exec();
}

// rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov
