//
//  FlameDetector.cpp
//  FlameDetection
//
//  Created by liberize on 14-4-5.
//  Copyright (c) 2014年 liberize. All rights reserved.
//

#include "FlameDetector.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>

extern CConfig mConfig;

FlameDetector::FlameDetector()
: mFrameCount(0)
, mFlameCount(0)
, mTrack(false)
{

}

int FlameDetector::detect(const Mat& frame)
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
        int result = mDecider.decide(mFrame, mTargetMap);
        finish = clock();

        if (result)
        {
            //mFlameCount++;
            m_Rect = Rect(mDecider.m_Rect.x + mConfig._config.cropX, mDecider.m_Rect.y + mConfig._config.cropY,
                          mDecider.m_Rect.width, mDecider.m_Rect.height);
            //StopCamera();

        }
        //cout << "duration: " << 1.0 * (finish - start) / CLOCKS_PER_SEC << endl;
        //cout << "frame: " << (mFrameCount - SKIP_FRAME_COUNT) << ", flame: " << mFlameCount << endl;
        //cout << "flame: " << mFlameCount << endl;
        //cout << "detection rate: " << 1.0 * mFlameCount / (mFrameCount - SKIP_FRAME_COUNT) << endl;

        return result;
    }
    
    return false;
}
