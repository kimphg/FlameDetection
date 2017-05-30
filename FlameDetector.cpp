//
//  FlameDetector.cpp
//  FlameDetection
//
//  Created by liberize on 14-4-5.
//  Copyright (c) 2014年 liberize. All rights reserved.
//

#include "FlameDetector.h"

#include <QNetworkRequest>

extern CConfig mConfig;

FlameDetector::FlameDetector()
: mFrameCount(0)
, mFlameCount(0)
, mTrack(false)
{
}
void FlameDetector::StopCamera()
{


    QNetworkRequest(QUrl("http://service:12345678@192.168.100.100/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085000000"));


    QNetworkRequest(QUrl("http://service:12345678@192.168.100.101/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085000000"));
}
void FlameDetector::StartCamera()
{


    QNetworkRequest(QUrl("http://service:12345678@192.168.100.100/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085010000"));



    QNetworkRequest(QUrl("http://service:12345678@192.168.100.101/rcp.xml?command=0x09A5&type=P_OCTET&direction=WRITE&num=1&payload=0x800006011085010000"));
}
bool FlameDetector::detect(const Mat& frame)
{
    mFrame = frame;
    
    clock_t start, finish;
    if(++mFrameCount > SKIP_FRAME_COUNT)
    {        
        mTrack = true;
        start = clock();
        mFrameCount = SKIP_FRAME_COUNT + 1;
    }
    
    mExtractor.extract(mFrame, mTargetMap, mTrack);
    if (mTrack)
    {
        mAnalyzer.analyze(mFrame, mTargetMap);
        bool result = mDecider.decide(mFrame, mTargetMap);
        finish = clock();

        if (result)
        {
            //mFlameCount++;
            m_Rect = Rect(mDecider.m_Rect.x + mConfig._config.cropX, mDecider.m_Rect.y + mConfig._config.cropY,
                          mDecider.m_Rect.width, mDecider.m_Rect.height);
            StopCamera();

        }
        //cout << "duration: " << 1.0 * (finish - start) / CLOCKS_PER_SEC << endl;
        //cout << "frame: " << (mFrameCount - SKIP_FRAME_COUNT) << ", flame: " << mFlameCount << endl;
        //cout << "flame: " << mFlameCount << endl;
        //cout << "detection rate: " << 1.0 * mFlameCount / (mFrameCount - SKIP_FRAME_COUNT) << endl;

        return result;
    }
    
    return false;
}
