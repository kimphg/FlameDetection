#include "videowork.h"
#include <QTimer>
#include <QEventLoop>

#include <QThread>
#include <QDebug>



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
    VideoCapture mCapture("D:/video/Example.asf");
    //"E:/My Works/ANTT/2017/VideoRecord/15.avi"

    bool continueToDetect = true;
    int extraFrameCount = 0;

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
            mCapture >> m_Frame;
            if(m_Frame.empty())
                break;
            namedWindow("original");
            moveWindow("original", 0, 0);
            resize(m_Frame, m_Frame, cvSize(400, 300));
            imshow("original", m_Frame);

            if (continueToDetect)
            {
                if (mDetector.detect(m_Frame))
                {
    //                if (mSaveKeyFrame && !saveFrame())
    //                {
    //                    cout << "Save key frame failed." << endl;
    //                }
    //                if (mSaveVideo)
    //                {
    //                    continueToDetect = false;
    //                    continue;
    //                }

                    cout << "Flame detected." << endl;
                    //return STATUS_FLAME_DETECTED;
                }
            }
            else if (++extraFrameCount >= 80)
            {
                return;
            }

    #ifdef TRAIN_MODE
            if (trainComplete) {
                cout << "Train complete." << endl;
                break;
            }
    #endif
            if (waitKey(30) == 27) {
                cout << "User abort." << endl;
                break;
            }

            waitKey(50);
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
    m_mutex.unlock();

    emit finished();
}
