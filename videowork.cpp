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
void VideoWork::StartCamera(QString ipadr,int rate)
{

    //StopCamera(ipadr);
    //printf("start cam \n");
#ifdef TEST_MODE
    return;
#endif
    if(rate>9||rate<0)return;
    //qnam = new QNetworkAccessManager();
    reply = qnam->get(QNetworkRequest(QUrl("http://service:12345678@"
    +ipadr+"/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x8000060110850"+QString::number(rate)+"0000")));
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
    Sleeper::sleep(3);
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
void VideoWork::commonWork(std::string url, QString ipadr,std::string winName,int videoPosition)
{
    int scanRate = 3;
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
    namedWindow(winName);
    moveWindow(winName,mConfig._config.frmWidth*videoPosition,0);
    int frameCountDown=1;
    int autoRateCountDown=0;
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

                m_mutex.unlock();
                int res = mDetector.detect(mFrame);
                if(mDetector.mTargetMap.size())
                {
                    if(scanRate>1)
                    {
                        autoRateCountDown = 500;
                        scanRate=1;
                        StartCamera(ipadr,scanRate);
                    }
                }
                else
                {
                    if(scanRate==1)
                    {
                        if(autoRateCountDown)autoRateCountDown--;
                        else
                        {
                            scanRate=3;
                            StartCamera(ipadr,scanRate);
                        }
                    }
                }
                if (res)
                {
                    //StartCamera(ipadr);

                    if(res>mConfig._config.alarmLevel)
                    {
                        videoHandler->ActivateAlarm();
                        StartCamera(ipadr,0);
                        frameCountDown = 2000;
                        if(saveFrame())
                        {

                            videoHandler->ActivateAlarm();
                            cout << "Flame detected." << endl;
                        }

                    }
                    else
                    {
                        StartCamera(ipadr,1);
                    }

                }
                else{
                    if(frameCountDown>-1)frameCountDown--;
                    if(frameCountDown==0)

                    {
                        //StopCamera(ipadr);
                        StartCamera(ipadr,scanRate);
                    }
                }
            }
            for (map<int, Target>::iterator it = mDetector.mTargetMap.begin(); it != mDetector.mTargetMap.end(); it++) {
                cv::Rect rect = it->second.region.rect;
                rect.x+=mROI.x;
                rect.y+=mROI.y;
                rectangle(m_Frame, rect, Scalar(255, 255, 0));
            }
            if(frameCountDown>0)
            {
                cv::putText(m_Frame,(QString::fromUtf8("Alarm level: ")
                                                 +QString::number(mConfig._config.alarmLevel)
                                                 +QString::fromUtf8(" |Scan rate:slow ")
                                                 ).toStdString(),
                                        cvPoint(150, 20), CV_FONT_HERSHEY_SIMPLEX, 0.5, cvScalar(255, 255, 0));
            }
            else cv::putText(m_Frame,(QString::fromUtf8("Alarm level: ")
                                 +QString::number(mConfig._config.alarmLevel)
                                 +QString::fromUtf8(" |Scan rate: ")
                                 +QString::number(scanRate)
                                 ).toStdString(),
                        cvPoint(150, 20), CV_FONT_HERSHEY_SIMPLEX, 0.5, cvScalar(255, 255, 0));
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
            else if (nKey ==45)
            {
                if(scanRate>1)scanRate--;
                StartCamera(ipadr,scanRate);
            }
            else if (nKey ==43)
            {
                if(scanRate<9)scanRate++;
                StartCamera(ipadr,scanRate);
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
void VideoWork::doWork()
{

    std::string url = mConfig._config.strCamUrl;
    QString ipadr = "192.168.100.100";
    std::string winName = "Camera-01";
    videoHandler->mVideoChannel = 1;
    int videoPosition = 0;
    commonWork(url,  ipadr, winName, videoPosition);

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
    videoHandler->mVideoChannel = 2;
    int videoPosition = 1;
    commonWork(url,  ipadr, winName, videoPosition);
}
void VideoWork::moveUp(QString ipadr)
{
#ifdef TEST_MODE
    return;
#endif
    //qnam = new QNetworkAccessManager();
    reply = qnam->get(QNetworkRequest(QUrl("http://service:12345678@"
    +ipadr+"/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085008700")));
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
    +ipadr+"/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085000700")));
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
    +ipadr+"/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085070000")));
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
    +ipadr+"/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085870000")));
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}
void VideoWork::doWork3()
{

    std::string url = mConfig._config.strCamUrl3;
    QString ipadr = "192.168.100.102";
    std::string winName = "Camera-03";
    videoHandler->mVideoChannel = 3;
    int videoPosition = 2;
    commonWork(url,  ipadr, winName, videoPosition);

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
    //if ((mDetector.m_Rect.y + mDetector.m_Rect.height) >= (mConfig._config.frmHeight - mConfig._config.cropY - 2))
        //return false;
    for (map<int, Target>::iterator it = mDetector.mTargetMap.begin(); it != mDetector.mTargetMap.end(); it++)
    {
        if((it->second.isSaved==false)&&(it->second.isFlame==true))
        {
            it->second.isSaved=true;
            string fileName;
            getCurTime(fileName);
            fileName += ".jpg";
            cout << "Saving key frame to '" << fileName << "'." << endl;
            //printf("times: %d\n",it->second.times);
             imwrite("C:\\FlameDetector\\" +fileName, m_Frame);
             break;
        }
    }
    return true;

}
