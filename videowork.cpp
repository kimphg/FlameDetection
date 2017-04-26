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
    VideoCapture mCapture("E:/My Works/ANTT/2017/VideoRecord/15.avi");
    //"E:/My Works/ANTT/2017/VideoRecord/15.avi"

    bool continueToDetect = true;
    int extraFrameCount = 0;
    Mat     mFrame;

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
            mCapture >> mFrame;
            if(mFrame.empty())
                break;
//            namedWindow("original");
//            moveWindow("original", 0, 0);
            m_mutex.lock();

            m_Frame = mFrame.clone();

            m_mutex.unlock();

 //           imshow("original", mFrame);


//            if (continueToDetect)
//            {
//                if (mDetector.detect(m_Frame))
//                {
//    //                if (mSaveKeyFrame && !saveFrame())
//    //                {
//    //                    cout << "Save key frame failed." << endl;
//    //                }
//    //                if (mSaveVideo)
//    //                {
//    //                    continueToDetect = false;
//    //                    continue;
//    //                }

//                    cout << "Flame detected." << endl;
//                    //return STATUS_FLAME_DETECTED;
//                }
//            }
//            else if (++extraFrameCount >= 80)
//            {
//                return;
//            }

//    #ifdef TRAIN_MODE
//            if (trainComplete) {
//                cout << "Train complete." << endl;
//                break;
//            }
//    #endif
//            if (waitKey(30) == 27) {
//                cout << "User abort." << endl;
//                break;
//            }

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
    m_mutex.unlock();

    emit finished();
}
