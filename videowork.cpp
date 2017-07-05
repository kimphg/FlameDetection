#include "videowork.h"
#include <QTimer>
#include <QEventLoop>

#include <QThread>
#include <QDebug>

#include "VideoHandler.h"

extern CConfig mConfig;
extern VideoHandler* videoHandler;
//int frameCountMin = 2;

VideoWork::VideoWork(QObject *parent) : QObject(parent)
{
    m_working =false;
    m_abort = false;
    qnam = new QNetworkAccessManager(this);
}
void VideoWork::StopCamera(QString ipadr)
{
    //StartCamera(ipadr);
    printf("stop cam \n");
#ifdef TEST_MODE
    return;
#endif
    reply = qnam->get(QNetworkRequest(QUrl("http://service:12345678@"
    +ipadr+"/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085000000")));
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}
void VideoWork::StartCamera(QString ipadr)
{
    //StopCamera(ipadr);
    printf("start cam \n");
#ifdef TEST_MODE
    return;
#endif
    //qnam = new QNetworkAccessManager();
    reply = qnam->get(QNetworkRequest(QUrl("http://service:12345678@"
    +ipadr+"/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085010000")));
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}
void VideoWork::setTilt(QString ipadr,int angle)
{
    switch(angle)
    {
    case 0:
        reply = qnam->get(QNetworkRequest(QUrl("http://service:12345678@"
        +ipadr+"/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x8100060113037700")));
        break;
    case 1:
        reply = qnam->get(QNetworkRequest(QUrl("http://service:12345678@"
        +ipadr+"/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x8100060113037300")));
        break;
    case 2:
        reply = qnam->get(QNetworkRequest(QUrl("http://service:12345678@"
        +ipadr+"/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x8100060113036f00")));
        break;
    default:
        break;

    }
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
//http://160.10.39.80/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x8100060113037600

}

void VideoWork::requestWork()
{
    m_mutex.lock();
    m_working = true;
    m_abort = false;
    //qDebug()<<"Connect to Camera...";
    m_mutex.unlock();
    emit workRequested();
}

void VideoWork::abort()
{
    m_mutex.lock();
    if (m_working) {
        m_abort = true;
        qDebug()<<"Request worker aborting in Thread "<<thread()->currentThreadId();
    }
    m_mutex.unlock();
}

void VideoWork::onTimer()
{
//    mDetector.StartCamera(QString::fromStdString(mConfig._config.strCamUrl));
//    mDetector.StartCamera(QString::fromStdString(mConfig._config.strCamUrl2));
//    mDetector.StartCamera(QString::fromStdString(mConfig._config.strCamUrl));
//    mDetector.StartCamera(QString::fromStdString(mConfig._config.strCamUrl2));
//    mDetector.StartCamera(QString::fromStdString(mConfig._config.strCamUrl));
//    mDetector.StartCamera(QString::fromStdString(mConfig._config.strCamUrl2));
    printf("\ncommand start");
    //_flushall();
    StopCamera("192.168.100.101");
    StopCamera("192.168.100.100");
    StopCamera("192.168.100.101");
    StopCamera("192.168.100.100");
    StopCamera("192.168.100.101");
    StopCamera("192.168.100.100");
    printf("\ncommand sent");
    //_flushall();
}
void VideoWork::doWork()
{

    std::string url = mConfig._config.strCamUrl;
    QString ipadr = "192.168.100.100";
    std::string winName = "Camera-01";

    VideoCapture mCapture(url);
    Mat mFrame;
    Rect mROI(mConfig._config.cropX, mConfig._config.cropY, 600 - (2*mConfig._config.cropX),
                 500 - (2*mConfig._config.cropY));
    m_IsFinished = false;

    if (!mCapture.isOpened())
    {
        mCapture.release();
        cout << "Capture video fail!" << endl;
        // Set _working to false, meaning the process can't be aborted anymore.
        m_mutex.lock();
        m_working = false;
        m_IsFinished = true;
        m_mutex.unlock();

        emit finished();
    }

    //qDebug()<<"Connected to Camera - 02...";
    namedWindow(winName);
    moveWindow(winName,mConfig._config.frmWidth,0);
    int flameRecently=1;
    bool isMoving = false;
    while(true)
    {
        // Checks if the process should be aborted
        m_mutex.lock();
        bool abort = m_abort;
        m_mutex.unlock();
        if (abort)
        {
            qDebug()<<"Request worker aborting in Thread "<<thread()->currentThreadId();
            break;
        }
        try
        {

            if(!mCapture.read(mFrame))
            {
                m_mutex.lock();
                cout << "Read frame fail!" << endl;
                m_IsFinished = true;
                mCapture.release();
                m_working = false;
                m_mutex.unlock();
                emit finished();
                break;
            }
            resize(mFrame, mFrame, cvSize(600, 500));
            mFrame.copyTo(m_Frame);

#ifdef MODE_GRAYSCALE
            cv::cvtColor(mFrame,mFrame, CV_BGRA2GRAY);
#endif
            mFrame = mFrame(mROI);

            if (true)//xu ly 3 frame 1 lan
            {
                m_mutex.lock();
                videoHandler->mVideoChannel = 2;
                m_mutex.unlock();
                int res = mDetector.detect(mFrame);
                if(res>mConfig._config.alarmLevel)videoHandler->ActivateAlarm();
                if (res>1)
                {
                    StartCamera(ipadr);
                    StopCamera(ipadr);
                    flameRecently = 3000;
                    if(saveFrame())
                    {
                        videoHandler->ActivateAlarm();
                        cout << "Flame detected." << endl;
                    }

                }
                else
                {
                    if(flameRecently>-1)flameRecently--;
                    if(flameRecently==0)

                    {
                        StopCamera(ipadr);
                        StartCamera(ipadr);
                    }
                }
            }
            for (map<int, Target>::iterator it = mDetector.mTargetMap.begin(); it != mDetector.mTargetMap.end(); it++) {
                cv::Rect rect = it->second.region.rect;
                rect.x+=mROI.x;
                rect.y+=mROI.y;
                rectangle(m_Frame, rect, Scalar(255, 255, 0));
            }
            cv::putText(m_Frame,(QString::fromUtf8("Alarm level: ")+QString::number(mConfig._config.alarmLevel)).toStdString(),
                        cvPoint(250, 20), CV_FONT_HERSHEY_SIMPLEX, 0.5, cvScalar(255, 255, 0));
            resize(m_Frame, m_Frame, cvSize(mConfig._config.frmWidth, mConfig._config.frmHeight));
            imshow(winName, m_Frame);

#ifdef TEST_MODE
            int nKey = waitKey(40);
#else
            int nKey = waitKey(10);
#endif
            if ((nKey >= 49) && (nKey < 58))
            {
                mConfig._config.alarmLevel = (nKey - 48);
                mConfig.SaveXmlFile();
            }

            else if (nKey == 32)
                resetProgram();
            else if (nKey == 27)
                closeProgram();
            else if (nKey =='w')
            {
                moveUp(ipadr);
                isMoving = true;
            }
            else if (nKey =='s')
            {
                moveDown(ipadr);
                isMoving = true;
            }
            else if (nKey =='a')
            {
                moveLeft(ipadr);
                isMoving = true;
            }
            else if (nKey =='d')
            {
                moveRight(ipadr);
                isMoving = true;
            }
            else if (isMoving)
            {
                StopCamera(ipadr);
                isMoving = false;
            }

           // phan code duoc bao ve
        }catch(...)
        {
            continue;
          // phan code de xu ly bat ky kieu ngoai le nao
        }

    }
    // ...
    // Set _working to false, meaning the process can't be aborted anymore.
    m_mutex.lock();
    mCapture.release();
    m_working = false;
    m_IsFinished = true;
    m_mutex.unlock();
    emit finished();
}
void VideoWork::resetProgram()
{
    std::system("reset.bat");
}

void VideoWork::closeProgram()
{
    std::system("closeApp.bat");
}

void VideoWork::doWork2()
{    
    std::string url = mConfig._config.strCamUrl2;
    QString ipadr = "192.168.100.101";
    std::string winName = "Camera-02";

    VideoCapture mCapture(url);
    Mat mFrame;
    Rect mROI(mConfig._config.cropX, mConfig._config.cropY, 600 - (2*mConfig._config.cropX),
                 500 - (2*mConfig._config.cropY));
    m_IsFinished = false;

    if (!mCapture.isOpened())
    {
        mCapture.release();
        cout << "Capture video fail!" << endl;
        // Set _working to false, meaning the process can't be aborted anymore.
        m_mutex.lock();
        m_working = false;
        m_IsFinished = true;
        m_mutex.unlock();

        emit finished();
    }

    //qDebug()<<"Connected to Camera - 02...";
    namedWindow(winName);
    moveWindow(winName,mConfig._config.frmWidth,0);
    int flameRecently=1;
    bool isMoving = false;
    while(true)
    {
        // Checks if the process should be aborted
        m_mutex.lock();
        bool abort = m_abort;
        m_mutex.unlock();
        if (abort)
        {
            qDebug()<<"Request worker aborting in Thread "<<thread()->currentThreadId();
            break;
        }
        try
        {

            if(!mCapture.read(mFrame))
            {
                m_mutex.lock();
                cout << "Read frame fail!" << endl;
                m_IsFinished = true;
                mCapture.release();
                m_working = false;
                m_mutex.unlock();
                emit finished();
                break;
            }
            resize(mFrame, mFrame, cvSize(600, 500));
            mFrame.copyTo(m_Frame);

#ifdef MODE_GRAYSCALE
            cv::cvtColor(mFrame,mFrame, CV_BGRA2GRAY);
#endif
            mFrame = mFrame(mROI);

            if (true)//xu ly 3 frame 1 lan
            {
                m_mutex.lock();
                videoHandler->mVideoChannel = 2;                
                m_mutex.unlock();
                int res = mDetector.detect(mFrame);
                if(res>mConfig._config.alarmLevel)videoHandler->ActivateAlarm();
                if (res>1)
                {
                    StartCamera(ipadr);
                    StopCamera(ipadr);
                    flameRecently = 3000;
                    if(saveFrame())
                    {
                        videoHandler->ActivateAlarm();
                        cout << "Flame detected." << endl;
                    }

                }
                else
                {
                    if(flameRecently>-1)flameRecently--;
                    if(flameRecently==0)

                    {
                        StopCamera(ipadr);
                        StartCamera(ipadr);
                    }
                }
            }
            for (map<int, Target>::iterator it = mDetector.mTargetMap.begin(); it != mDetector.mTargetMap.end(); it++) {
                cv::Rect rect = it->second.region.rect;
                rect.x+=mROI.x;
                rect.y+=mROI.y;
                rectangle(m_Frame, rect, Scalar(255, 255, 0));
            }
            cv::putText(m_Frame,(QString::fromUtf8("Alarm level: ")+QString::number(mConfig._config.alarmLevel)).toStdString(),
                        cvPoint(250, 20), CV_FONT_HERSHEY_SIMPLEX, 0.5, cvScalar(255, 255, 0));
            resize(m_Frame, m_Frame, cvSize(mConfig._config.frmWidth, mConfig._config.frmHeight));
            imshow(winName, m_Frame);

#ifdef TEST_MODE
            int nKey = waitKey(40);
#else
            int nKey = waitKey(10);
#endif
            if ((nKey >= 49) && (nKey < 58))
            {
                mConfig._config.alarmLevel = (nKey - 48);
                mConfig.SaveXmlFile();
            }

            else if (nKey == 32)
                resetProgram();
            else if (nKey == 27)
                closeProgram();
            else if (nKey =='w')
            {
                moveUp(ipadr);
                isMoving = true;
            }
            else if (nKey =='s')
            {
                moveDown(ipadr);
                isMoving = true;
            }
            else if (nKey =='a')
            {
                moveLeft(ipadr);
                isMoving = true;
            }
            else if (nKey =='d')
            {
                moveRight(ipadr);
                isMoving = true;
            }
            else if (isMoving)
            {
                StopCamera(ipadr);
                isMoving = false;
            }

           // phan code duoc bao ve
        }catch(...)
        {
            continue;
          // phan code de xu ly bat ky kieu ngoai le nao
        }

    }
    // ...
    // Set _working to false, meaning the process can't be aborted anymore.
    m_mutex.lock();
    mCapture.release();
    m_working = false;
    m_IsFinished = true;
    m_mutex.unlock();
    emit finished();
}
void VideoWork::moveUp(QString ipadr)
{
#ifdef TEST_MODE
    return;
#endif
    //qnam = new QNetworkAccessManager();
    reply = qnam->get(QNetworkRequest(QUrl("http://service:12345678@"
    +ipadr+"/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085008200")));
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}
void VideoWork::moveDown(QString ipadr)
{
#ifdef TEST_MODE
    return;
#endif
    //qnam = new QNetworkAccessManager();
    reply = qnam->get(QNetworkRequest(QUrl("http://service:12345678@"
    +ipadr+"/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085000200")));
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}
void VideoWork::moveLeft(QString ipadr)
{
#ifdef TEST_MODE
    return;
#endif
    //qnam = new QNetworkAccessManager();
    reply = qnam->get(QNetworkRequest(QUrl("http://service:12345678@"
    +ipadr+"/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085020000")));
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}
void VideoWork::moveRight(QString ipadr)
{
#ifdef TEST_MODE
    return;
#endif
    //qnam = new QNetworkAccessManager();
    reply = qnam->get(QNetworkRequest(QUrl("http://service:12345678@"
    +ipadr+"/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085820000")));
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}
void VideoWork::doWork3()
{
    std::string url = mConfig._config.strCamUrl3;
    QString ipadr = "192.168.100.102";
    std::string winName = "Camera-03";

    VideoCapture mCapture(url);
    Mat mFrame;
    Rect mROI(mConfig._config.cropX, mConfig._config.cropY, 600 - (2*mConfig._config.cropX),
                 500 - (2*mConfig._config.cropY));
    m_IsFinished = false;

    if (!mCapture.isOpened())
    {
        mCapture.release();
        cout << "Capture video fail!" << endl;
        // Set _working to false, meaning the process can't be aborted anymore.
        m_mutex.lock();
        m_working = false;
        m_IsFinished = true;
        m_mutex.unlock();

        emit finished();
    }

    //qDebug()<<"Connected to Camera - 02...";
    namedWindow(winName);
    moveWindow(winName,mConfig._config.frmWidth,0);
    int flameRecently=1;
    bool isMoving = false;
    while(true)
    {
        // Checks if the process should be aborted
        m_mutex.lock();
        bool abort = m_abort;
        m_mutex.unlock();
        if (abort)
        {
            qDebug()<<"Request worker aborting in Thread "<<thread()->currentThreadId();
            break;
        }
        try
        {

            if(!mCapture.read(mFrame))
            {
                m_mutex.lock();
                cout << "Read frame fail!" << endl;
                m_IsFinished = true;
                mCapture.release();
                m_working = false;
                m_mutex.unlock();
                emit finished();
                break;
            }
            resize(mFrame, mFrame, cvSize(600, 500));
            mFrame.copyTo(m_Frame);

#ifdef MODE_GRAYSCALE
            cv::cvtColor(mFrame,mFrame, CV_BGRA2GRAY);
#endif
            mFrame = mFrame(mROI);

            if (true)//xu ly 3 frame 1 lan
            {
                m_mutex.lock();
                videoHandler->mVideoChannel = 2;
                m_mutex.unlock();
                int res = mDetector.detect(mFrame);
                if(res>mConfig._config.alarmLevel)videoHandler->ActivateAlarm();
                if (res>1)
                {
                    StartCamera(ipadr);
                    StopCamera(ipadr);
                    flameRecently = 3000;
                    if(saveFrame())
                    {
                        videoHandler->ActivateAlarm();
                        cout << "Flame detected." << endl;
                    }

                }
                else
                {
                    if(flameRecently>-1)flameRecently--;
                    if(flameRecently==0)

                    {
                        StopCamera(ipadr);
                        StartCamera(ipadr);
                    }
                }
            }
            for (map<int, Target>::iterator it = mDetector.mTargetMap.begin(); it != mDetector.mTargetMap.end(); it++) {
                cv::Rect rect = it->second.region.rect;
                rect.x+=mROI.x;
                rect.y+=mROI.y;
                rectangle(m_Frame, rect, Scalar(255, 255, 0));
            }
            cv::putText(m_Frame,(QString::fromUtf8("Alarm level: ")+QString::number(mConfig._config.alarmLevel)).toStdString(),
                        cvPoint(250, 20), CV_FONT_HERSHEY_SIMPLEX, 0.5, cvScalar(255, 255, 0));
            resize(m_Frame, m_Frame, cvSize(mConfig._config.frmWidth, mConfig._config.frmHeight));
            imshow(winName, m_Frame);

#ifdef TEST_MODE
            int nKey = waitKey(40);
#else
            int nKey = waitKey(10);
#endif
            if ((nKey >= 49) && (nKey < 58))
            {
                mConfig._config.alarmLevel = (nKey - 48);
                mConfig.SaveXmlFile();
            }

            else if (nKey == 32)
                resetProgram();
            else if (nKey == 27)
                closeProgram();
            else if (nKey =='w')
            {
                moveUp(ipadr);
                isMoving = true;
            }
            else if (nKey =='s')
            {
                moveDown(ipadr);
                isMoving = true;
            }
            else if (nKey =='a')
            {
                moveLeft(ipadr);
                isMoving = true;
            }
            else if (nKey =='d')
            {
                moveRight(ipadr);
                isMoving = true;
            }
            else if (isMoving)
            {
                StopCamera(ipadr);
                isMoving = false;
            }

           // phan code duoc bao ve
        }catch(...)
        {
            continue;
          // phan code de xu ly bat ky kieu ngoai le nao
        }

    }
    // ...
    // Set _working to false, meaning the process can't be aborted anymore.
    m_mutex.lock();
    mCapture.release();
    m_working = false;
    m_IsFinished = true;
    m_mutex.unlock();
    emit finished();
}


bool VideoWork::saveFrame()
{
    rectangle(m_Frame, mDetector.m_Rect, Scalar(0, 255, 0));
//    rectangle(mOrgFrame, Rect(mConfig._config.cropX, mConfig._config.cropY, mConfig._config.frmWidth - (2*mConfig._config.cropX), mConfig._config.frmHeight
//                              - (2*mConfig._config.cropY)), Scalar(0, 0, 255));

//    if (mDetector.m_Rect.x < (mConfig._config.frmWidth*1/3))
//        return false;
//    if ((mDetector.m_Rect.x +mDetector.m_Rect.width )> (mConfig._config.frmWidth*2/3))
//        return false;
    if ((mDetector.m_Rect.y + mDetector.m_Rect.height) >= (mConfig._config.frmHeight - mConfig._config.cropY - 2))
        return false;

    // save detected frame to jpg
    string fileName;
    getCurTime(fileName);
    fileName += ".jpg";
    cout << "Saving key frame to '" << fileName << "'." << endl;
    //printf("times: %d\n",it->second.times);
    return imwrite("C:\\FlameDetector\\" +fileName, m_Frame);
}
