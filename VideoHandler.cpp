//
//  VideoHandler.cpp
//  FlameDetection
//
//  Created by liberize on 14-4-6.
//  Copyright (c) 2014ๅนด liberize. All rights reserved.
//

#include "VideoHandler.h"

extern CConfig mConfig;

VideoHandler::VideoHandler(int device, bool saveKeyFrame, bool saveVideo)
: mCapture(device)
, mSaveKeyFrame(saveKeyFrame)
, mSaveVideo(saveVideo)
, mFromCam(true)
, mVideoFPS(0)
{
    if (mCapture.isOpened()) {
        mVideoFPS = mCapture.get(CV_CAP_PROP_FPS);
        if (mVideoFPS == 0) {
            mVideoFPS = 8.0;
        }
    }
}

VideoHandler::VideoHandler(const string& file, bool saveKeyFrame)
: mCapture(file)
, mSaveKeyFrame(saveKeyFrame)
, mFromCam(false)
, mVideoFPS(0)
{
    if (mCapture.isOpened()) {
        mVideoFPS = mCapture.get(CV_CAP_PROP_FPS);
        assert(mVideoFPS != 0);
    }
}

int VideoHandler::handle()
{
    if (!mCapture.isOpened()) {
        return STATUS_OPEN_CAP_FAILED;
    }

    bool continueToDetect = true;
    int extraFrameCount = 0;

//    mCapture.set(CV_CAP_PROP_FRAME_WIDTH, mConfig._config.frmWidth);
//    mCapture.set(CV_CAP_PROP_FRAME_HEIGHT, mConfig._config.frmHeight);


    while (continueToDetect)
    {
        if (!mCapture.read(mFrame))
        {
            cout << (mFromCam ? "Camera disconnected." : "Video file ended.") << endl;
            break;
        }
        if (mFrame.empty()) continue;
#ifdef MODE_GRAYSCALE
        cv::cvtColor(mFrame,mFrame, CV_BGRA2GRAY);
#endif
        imshow("original", mFrame);
        resize(mFrame, mFrame, cvSize(mConfig._config.frmWidth, mConfig._config.frmHeight));

        if (mSaveVideo && !saveVideo())
        {
            cout << "Save video failed." << endl;
            mSaveVideo = false;
        }

        if (true)//xu ly 3 frame 1 lan
        {
            if (mDetector.detect(mFrame))
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
    string fileName;
    getCurTime(fileName);
    fileName += ".jpg";
    cout << "Saving key frame to '" << fileName << "'." << endl;

    return imwrite(fileName, mFrame);
}

bool VideoHandler::saveVideo()
{
    if (mSaveVideoFile.empty()) {
        getCurTime(mSaveVideoFile);
        mSaveVideoFile += ".mov";
        cout << "Saving video to '" << mSaveVideoFile << "'." << endl;

        // in Mac OS X, only 'mp4v' is supported
        int fourcc = CV_FOURCC('m', 'p', '4', 'v');
        Size size = Size((int)mCapture.get(CV_CAP_PROP_FRAME_WIDTH),
                         (int)mCapture.get(CV_CAP_PROP_FRAME_HEIGHT));

        mWriter.open(mSaveVideoFile, fourcc, mVideoFPS, size, true);
    }

    if (!mWriter.isOpened()) {
        return false;
    }

    mWriter << mFrame;
    return true;
}
