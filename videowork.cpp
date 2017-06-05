#include "videowork.h"
#include <QTimer>
#include <QEventLoop>

#include <QThread>
#include <QDebug>

#include "VideoHandler.h"

extern CConfig mConfig;
extern VideoHandler* videoHandler;

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

    QNetworkRequest request(QUrl("http://service:12345678@"+ipadr+"/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085000000"));
    reply = qnam->get(request);

    //QNetworkRequest rq(QUrl("http://service:12345678@192.168.100.101/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085000000"));
    //reply = qnam->get(rq);
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
}
void VideoWork::StartCamera(QString ipadr)
{
    //StopCamera(ipadr);
    printf("start cam \n");

    //qnam = new QNetworkAccessManager();
    reply = qnam->get(QNetworkRequest(QUrl("http://service:12345678@"
    +ipadr+"/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085010000")));
    //QNetworkRequest rq(QUrl("http://service:12345678@192.168.100.101/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085010000"));
    //reply = qnam->get(rq);
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
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

    //onTimer();
    VideoCapture mCapture(url);
    Mat mFrame;    
    Rect mROI(mConfig._config.cropX, mConfig._config.cropY, mConfig._config.frmWidth - (2*mConfig._config.cropX),
                 mConfig._config.frmHeight - (2*mConfig._config.cropY));
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

    qDebug()<<"Connected to Camera - 01...";
    int flameRecently = 1;
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

            resize(mFrame, mFrame, cvSize(mConfig._config.frmWidth, mConfig._config.frmHeight));
            mFrame.copyTo(m_Frame);

#ifdef MODE_GRAYSCALE
            cv::cvtColor(mFrame,mFrame, CV_BGRA2GRAY);
#endif
            mFrame = mFrame(mROI);

            if (true)//xu ly 3 frame 1 lan
            {
                m_mutex.lock();
                videoHandler->mVideoChannel = 1;
                m_mutex.unlock();
                int res = mDetector.detect(mFrame);
                if(res>6)videoHandler->ActivateAlarm();
                if (res>4)
                {
                    StartCamera("192.168.100.100");
                    StopCamera("192.168.100.100");

                    flameRecently = 20;
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
                        StopCamera("192.168.100.100");
                        StartCamera("192.168.100.100");
                    }
                }

            }            

            imshow("Camera-01", m_Frame);

            waitKey(10);
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
    m_working = false;
    m_IsFinished = true;
    m_mutex.unlock();

    emit finished();
}

void VideoWork::doWork2()
{    
    std::string url = mConfig._config.strCamUrl2;
    VideoCapture mCapture(url);

    Mat mFrame;
    Rect mROI(mConfig._config.cropX, mConfig._config.cropY, mConfig._config.frmWidth - (2*mConfig._config.cropX),
                 mConfig._config.frmHeight - (2*mConfig._config.cropY));
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

    qDebug()<<"Connected to Camera - 02...";
    int flameRecently=1;
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
            resize(mFrame, mFrame, cvSize(mConfig._config.frmWidth, mConfig._config.frmHeight));
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
                videoHandler->mVideoChannel = 1;
                m_mutex.unlock();
                int res = mDetector.detect(mFrame);
                if(res>6)videoHandler->ActivateAlarm();
                if (res>4)
                {
                    StartCamera("192.168.100.101");
                    StopCamera("192.168.100.101");
                    flameRecently = 20;
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
                        StopCamera("192.168.100.101");
                        StartCamera("192.168.100.101");
                    }
                }
            }

            imshow("Camera-02", m_Frame);

            waitKey(10);
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

void VideoWork::doWork3()
{
    std::string url = mConfig._config.strCamUrl3;
    VideoCapture mCapture(url);

    Mat mFrame;
    Rect mROI(mConfig._config.cropX, mConfig._config.cropY, mConfig._config.frmWidth - (2*mConfig._config.cropX),
                 mConfig._config.frmHeight - (2*mConfig._config.cropY));
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

    qDebug()<<"Connected to Camera - 03...";
    int flameRecently = 1;
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
            resize(mFrame, mFrame, cvSize(mConfig._config.frmWidth, mConfig._config.frmHeight));
            mFrame.copyTo(m_Frame);

#ifdef MODE_GRAYSCALE
            cv::cvtColor(mFrame,mFrame, CV_BGRA2GRAY);
#endif
            mFrame = mFrame(mROI);
            if (true)//xu ly 3 frame 1 lan
            {
                m_mutex.lock();
                videoHandler->mVideoChannel = 1;
                m_mutex.unlock();
                int res = mDetector.detect(mFrame);
                if(res>6)videoHandler->ActivateAlarm();
                if (res>4)
                {
                    StartCamera("192.168.100.102");
                    StopCamera("192.168.100.102");
                    flameRecently = 50;
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
                        StopCamera("192.168.100.102");
                        StartCamera("192.168.100.102");
                    }
                }
            }

            imshow("Camera-03", m_Frame);

            waitKey(10);
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
