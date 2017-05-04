//
//  VideoHandler.cpp
//  FlameDetection
//
//  Created by liberize on 14-4-6.
//  Copyright (c) 2014ๅนด liberize. All rights reserved.
//

#include "VideoHandler.h"

extern CConfig mConfig;

VideoHandler::VideoHandler(int device)
: mCapture(device)
{
}

VideoHandler::VideoHandler(const string& file)
: mCapture(file)
{
}
void VideoHandler::ActivateAlarm()
{
    unsigned char message[5];
    message[0]=0xff;
    message[1]=0xff;
    message[2]=0xff;
    message[3]= 1;//mConfig._config.alarmNumber;
    message[4]=0xff;
    alarmSocket->writeDatagram((char*)&message[0],5, QHostAddress("192.168.100.255") , 8888);
}
void VideoHandler::DeactivateAlarm()
{
    unsigned char message[5];
    message[0]= 0xff;
    message[1]= 0xff;
    message[2]= 0xff;
    message[3]= mConfig._config.alarmNumber;
    message[4]= 0x00;
    alarmSocket->writeDatagram((char*)&message[0],5, QHostAddress("192.168.100.255") , 8888);
}
int VideoHandler::handle()
{
   // TCPClient client;
    //client.start("127.0.0.1", 8888);
    alarmSocket = new QUdpSocket();
    bool continueToDetect = true;
    int extraFrameCount = 0;

    Rect mROI(mConfig._config.cropX, mConfig._config.cropY, mConfig._config.frmWidth - (2*mConfig._config.cropX),
                 mConfig._config.frmHeight - (2*mConfig._config.cropY));

    QSound sound("alarm.wav");
    sound.setLoops(15);


    // The thread and the worker are created in the constructor so it is always safe to delete them.
#ifdef MODE_MULTITHREAD
    mCapture.release();
    m_thread = new QThread();
    m_worker = new VideoWork();
    m_worker->moveToThread(m_thread);
    QObject::connect(m_worker, SIGNAL(workRequested()), m_thread, SLOT(start()));
    QObject::connect(m_thread, SIGNAL(started()), m_worker, SLOT(doWork()));
    QObject::connect(m_worker, SIGNAL(finished()), m_thread, SLOT(quit()), Qt::DirectConnection);    

    m_worker->abort();
    m_thread->wait();
    m_worker->requestWork();

    while (continueToDetect)
    {        
        m_worker->m_Frame.copyTo(mOrgFrame);
        if (mOrgFrame.empty())
            continue;

#ifdef MODE_GRAYSCALE
            cv::cvtColor(mOrgFrame, mFrame, CV_BGRA2GRAY);
#endif

            mFrame = mFrame(mROI);

        if (m_worker->m_IsFinished)
            return 0;

        if (true)
        {
            if (mDetector.detect(mFrame))
            {
                saveFrame();                
                if (sound.isFinished())
                    sound.play();
                cout << "Flame detected." << endl;
                this->ActivateAlarm();

                //return STATUS_FLAME_DETECTED;
            }
            //    namedWindow("result");
            //    moveWindow("result", 0, 0);
            imshow("result", mOrgFrame);
        }
        else if (++extraFrameCount >= MAX_EXTRA_FRAME_COUNT)
        {
            return STATUS_FLAME_DETECTED;
        }
#ifdef TRAIN_MODE
        if (trainComplete) {
            cout << "Train complete." << endl;
            break;
        }
#endif
        if (waitKey(WAIT_INTERVAL) == 27) {
            cout << "User abort." << endl;
            break;
        }
    }

    return STATUS_NO_FLAME_DETECTED;

    return 0;
#endif

    if (!mCapture.isOpened()) {
        return STATUS_OPEN_CAP_FAILED;
    }

    while (continueToDetect)
    {
        if (!mCapture.read(mOrgFrame))
        {
            cout << (mFromCam ? "Camera disconnected." : "Video file ended.") << endl;
            break;
        }

        resize(mOrgFrame, mOrgFrame, cvSize(mConfig._config.frmWidth, mConfig._config.frmHeight));
        imshow("original", mOrgFrame);

#ifdef MODE_GRAYSCALE
        cv::cvtColor(mOrgFrame,mFrame, CV_BGRA2GRAY);
#endif

        mFrame = mFrame(mROI);


        if (true)//xu ly 3 frame 1 lan
        {

            if (mDetector.detect(mFrame))
            {
                saveFrame();

                if (sound.isFinished())
                    sound.play();
                cout << "Flame detected." << endl;
                //return STATUS_FLAME_DETECTED;
            }

            imshow("result", mOrgFrame);

        }
        else if (++extraFrameCount >= MAX_EXTRA_FRAME_COUNT)
        {
            return STATUS_FLAME_DETECTED;
        }
#ifdef TRAIN_MODE
        if (trainComplete) {
            cout << "Train complete." << endl;
            break;
        }
#endif
        if (waitKey(WAIT_INTERVAL) == 27) {
            cout << "User abort." << endl;
            break;
        }
    }

    return STATUS_NO_FLAME_DETECTED;
}

bool VideoHandler::saveFrame()
{   
    rectangle(mOrgFrame, mDetector.m_Rect, Scalar(0, 255, 0));
    if (mDetector.m_Rect.x < (mConfig._config.frmWidth/3))
        return false;
    if (mDetector.m_Rect.x > (mConfig._config.frmWidth*2/3))
        return false;
    // save detected frame to jpg
    string fileName;
    getCurTime(fileName);
    fileName += ".jpg";
    cout << "Saving key frame to '" << fileName << "'." << endl;
    //printf("times: %d\n",it->second.times);
    return imwrite("C:\\FlameDetector\\" +fileName, mOrgFrame);
}
