//
//  VideoHandler.cpp
//  FlameDetection
//
//  Created by liberize on 14-4-6.
//  Copyright (c) 2014ๅนด liberize. All rights reserved.
//

#include "VideoHandler.h"

extern CConfig mConfig;
VideoWork       *m_worker = NULL;
VideoWork       *m_worker2 = NULL;
VideoWork       *m_worker3 = NULL;
QSound sound("alarm.wav");

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
    QUdpSocket      alarmSocket;
    if (sound.isFinished())
    sound.play();
    unsigned char message[5];
    message[0]=0xff;
    message[1]=0xff;
    message[2]=0xff;
    message[3]= mConfig._config.alarmNumber;
    message[4]=0xff;
    alarmSocket.writeDatagram((char*)&message[0],5, QHostAddress("192.168.100.255") , 8888);
}
void VideoHandler::DeactivateAlarm()
{
    QUdpSocket      alarmSocket;
    unsigned char message[5];
    message[0]= 0xff;
    message[1]= 0xff;
    message[2]= 0xff;
    message[3]= mConfig._config.alarmNumber;
    message[4]= 0x00;
    alarmSocket.writeDatagram((char*)&message[0],5, QHostAddress("192.168.100.255") , 8888);
}
void VideoHandler::onTimer()
{
//    printf("\ncommand start");
//    //QNetworkRequest request(QUrl("http://service:12345678@"+ipadr+"/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085000000"));
//    //reply = qnam->get(request);
//    QNetworkAccessManager qnam;
//    QNetworkRequest rq(QUrl("http://service:12345678@192.168.100.101/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085000000"));
//    QNetworkReply* reply = qnam.get(rq);
//    QEventLoop loop;
//    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
//    loop.exec();
//    printf("\ncommand sent");
//    flushall();
}
int VideoHandler::handle()
{
    //QTimer::singleShot(2000, this, SLOT(onTimer()));
   // TCPClient client;
    //client.start("127.0.0.1", 8888);
    mVideoChannel = 0;
    //alarmSocket = new QUdpSocket();
    bool continueToDetect = true;
    int extraFrameCount = 0;    

    Rect mROI(mConfig._config.cropX, mConfig._config.cropY, mConfig._config.frmWidth - (2*mConfig._config.cropX),
                 mConfig._config.frmHeight - (2*mConfig._config.cropY));

    sound.setLoops(15);

    // The thread and the worker are created in the constructor so it is always safe to delete them.
#ifdef MODE_MULTITHREAD
    mCapture.release();
    // first thread
    m_thread = new QThread();
    m_worker = new VideoWork();
    m_worker->moveToThread(m_thread);
    QObject::connect(m_worker, SIGNAL(workRequested()), m_thread, SLOT(start()));
    QObject::connect(m_thread, SIGNAL(started()), m_worker, SLOT(doWork()));
    QObject::connect(m_worker, SIGNAL(finished()), m_thread, SLOT(quit()), Qt::DirectConnection);
    m_worker->abort();
    m_thread->wait();
    m_worker->requestWork();

    // second thread
    m_thread2 = new QThread();
    m_worker2 = new VideoWork();
    m_worker2->moveToThread(m_thread2);
    QObject::connect(m_worker2, SIGNAL(workRequested()), m_thread2, SLOT(start()));
    QObject::connect(m_thread2, SIGNAL(started()), m_worker2, SLOT(doWork2()));
    QObject::connect(m_worker2, SIGNAL(finished()), m_thread2, SLOT(quit()), Qt::DirectConnection);
    m_worker2->abort();
    m_thread2->wait();
    m_worker2->requestWork();

    // third thread
    m_thread3 = new QThread();
    m_worker3 = new VideoWork();
    m_worker3->moveToThread(m_thread3);
    QObject::connect(m_worker3, SIGNAL(workRequested()), m_thread3, SLOT(start()));
    QObject::connect(m_thread3, SIGNAL(started()), m_worker3, SLOT(doWork3()));
    QObject::connect(m_worker3, SIGNAL(finished()), m_thread3, SLOT(quit()), Qt::DirectConnection);
    m_worker3->abort();
    m_thread3->wait();
    m_worker3->requestWork();

    return 3;
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
        //imshow("original", mOrgFrame);

#ifdef MODE_GRAYSCALE
        cv::cvtColor(mOrgFrame,mFrame, CV_BGRA2GRAY);
#endif

        mFrame = mFrame(mROI);


        if (true)//xu ly 3 frame 1 lan
        {

            if (mDetector.detect(mFrame))
            {
                if(saveFrame())
                {
//                    if (sound.isFinished())
//                    sound.play();
                    this->ActivateAlarm();
                    cout << "Flame detected." << endl;
                }
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
    return imwrite("C:\\FlameDetector\\" +fileName, mOrgFrame);
}
