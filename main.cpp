#include <QCoreApplication>
#include <QSharedMemory>

#include "common.h"
#include "VideoHandler.h"
#include "FlameDetector.h"
#include "Config.h"

#ifdef TRAIN_MODE
bool trainComplete = false;
#endif

VideoHandler* videoHandler = NULL;
CConfig mConfig;


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    a.processEvents();
    QSharedMemory shared("62d60669-bb94-4a94-88bb-b964890a7e04");
    if( !shared.create( 512, QSharedMemory::ReadWrite) )
    {
//        QMessageBox msgBox;
//        msgBox.setText( QObject::tr("Can't start more than one instance of TrackCam!") );
//        msgBox.setIcon( QMessageBox::Critical );
//        msgBox.exec();
        exit(0);
    }

    VideoHandler handler(mConfig._config.strCamUrl);

    videoHandler = &handler;

    int ret = handler.handle();

    switch (ret) {
        case VideoHandler::STATUS_FLAME_DETECTED:
            cout << "Flame detected." << endl;
            break;
        case VideoHandler::STATUS_OPEN_CAP_FAILED:
            cout << "Open capture failed." << endl;
            break;
        case VideoHandler::STATUS_NO_FLAME_DETECTED:
            cout << "No flame detected." << endl;
            break;
        default:
            cout << "Detecting flames..." << endl;
            break;
        }

    return a.exec();
}
