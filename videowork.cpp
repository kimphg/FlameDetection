#include "videowork.h"
#include <QTimer>
#include <QEventLoop>

#include <QThread>
#include <QDebug>

extern CConfig mConfig;

VideoWork::VideoWork(QObject *parent) : QObject(parent)
{
    m_working =false;
    m_abort = false;
}

void VideoWork::requestWork()
{
    m_mutex.lock();
    m_working = true;
    m_abort = false;
    qDebug()<<"Request worker start in Thread "<<thread()->currentThreadId();
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

void VideoWork::doWork()
{
    //VideoCapture mCapture("rtsp://192.168.0.253:554/stream1");
    VideoCapture mCapture("E:/My Works/ANTT/2017/VideoRecord/15.avi");
    Mat mFrame;    
    Rect mROI(mConfig._config.cropX, mConfig._config.cropY, mConfig._config.frmWidth - (2*mConfig._config.cropX),
                 mConfig._config.frmHeight - (2*mConfig._config.cropY));
    m_IsFinished = false;

    if (!mCapture.isOpened())
    {
        mCapture.release();

        // Set _working to false, meaning the process can't be aborted anymore.
        m_mutex.lock();
        m_working = false;
        m_IsFinished = true;
        m_mutex.unlock();

        emit finished();
    }

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
                m_IsFinished = true;
                mCapture.release();
                m_working = false;
                m_mutex.unlock();
                emit finished();
                break;
            }

            imshow("original", mFrame);

#ifdef MODE_GRAYSCALE
            cv::cvtColor(mFrame,mFrame, CV_BGRA2GRAY);
#endif
            resize(mFrame, mFrame, cvSize(mConfig._config.frmWidth, mConfig._config.frmHeight));

            mFrame = mFrame(mROI);

            m_mutex.lock();
            mFrame.copyTo(m_Frame);
            m_mutex.unlock();

            waitKey(40);
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
