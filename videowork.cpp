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

void VideoWork::doWork()
{
    VideoCapture mCapture(mConfig._config.strCamUrl);

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

                if (mDetector.detect(mFrame))
                {
                    if(saveFrame())
                    {                        
                        videoHandler->ActivateAlarm();
                        cout << "Flame detected." << endl;
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
    VideoCapture mCapture(mConfig._config.strCamUrl2);

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

                if (mDetector.detect(mFrame))
                {

                    if(saveFrame())
                    {
                        videoHandler->ActivateAlarm();
                        cout << "Flame detected." << endl;
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
    m_working = false;
    m_IsFinished = true;
    m_mutex.unlock();

    emit finished();
}

void VideoWork::doWork3()
{
    VideoCapture mCapture(mConfig._config.strCamUrl3);

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
                videoHandler->mVideoChannel = 3;
                m_mutex.unlock();

                if (mDetector.detect(mFrame))
                {

                    if(saveFrame())
                    {
                        videoHandler->ActivateAlarm();
                        cout << "Flame detected." << endl;
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
