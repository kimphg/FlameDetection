//
//  FeatureAnalyzer.cpp
//  FlameDetection
//
//  Created by liberize on 14-4-14.
//  Copyright (c) 2014年 liberize. All rights reserved.
//

#include "FeatureAnalyzer.h"
#include "VideoHandler.h"
#include "FlameDetector.h"
#include "TargetExtractor.h"
#include "videowork.h"

extern VideoHandler* videoHandler;
extern VideoWork *m_worker;
extern VideoWork *m_worker2;
extern VideoWork *m_worker3;
extern CConfig mConfig;

/**************** Feature ****************/

void Feature::calcColorFeature()
{
    // TODO: optimize this part, reduce extra work
    /*
    Mat hsv;
    cvtColor(mROI, hsv, CV_BGR2HSV_FULL);
    
    Mat temp(mROI.size(), CV_8UC3), mixed;
    Mat src[] = { mROI, mGray, hsv };
    int fromTo[] = { 2,0, 3,1, 5,2 };
    mixChannels(src, 3, &temp, 1, fromTo, 3);
    temp.convertTo(mixed, CV_64F);
    
    Scalar avg, stdDev;
    meanStdDev(mixed, avg, stdDev, mMask);
    Scalar var = stdDev.mul(stdDev);
    Mat temp1 = mixed - avg;
    Mat temp2 = temp1.mul(temp1);
    Scalar sk = mean(temp1.mul(temp2), mMask) / (var.mul(stdDev));
    Scalar ku = mean(temp2.mul(temp2), mMask) / (var.mul(var));
    
    Scalar stat[] = { avg, stdDev, sk, ku };
    for (int i = 0; i < 4; i++) {
        red[i] = stat[i][0];
        gray[i] = stat[i][1];
        saturation[i] = stat[i][2];
    }*/
}

void Feature::calcGeometryFeature(const Region& region)
{
    double new_circularity = 0;
    double new_squareness = 0;
    double new_roughness = 0;
    double aspectRatio ;
    double area ;
    const vector<ContourInfo*>& contours = region.contours;
    for (vector<ContourInfo*>::const_iterator it = contours.begin(); it == contours.begin(); it++) {//only first element
        const vector<Point>& contour = (*it)->contour;
        area = (*it)->area;
        double perimeter = arcLength(contour, true);
        RotatedRect minRect = minAreaRect(Mat(contour));
        vector<Point> hull;
        convexHull(contour, hull);
        double perimeterHull = arcLength(hull, true);
        double width = minRect.size.width, height = minRect.size.height;

        new_circularity = (4.0 * 3.141592654 * area / (perimeter * perimeter));
        new_squareness =  (area / (width * height));
        //new_aspectRatio += area * (1.0 * min(width, height) / max(width, height));
        new_roughness   =  (perimeterHull / perimeter);
        aspectRatio = width/height;
        if(aspectRatio<1)aspectRatio = 1.0/aspectRatio;
        assert(aspectRatio>=1);
        roughness +=(new_roughness-roughness)/5.0;
    }
    //update all vectors
    mAreaVec.push_back(area);
    aspectRatioVec.push_back(aspectRatio);
    circularityVec.push_back(new_circularity);
    squarenessVec.push_back(new_squareness);
    if (mAreaVec.size() > MAX_AREA_VEC_SIZE) {
        dataReady = true;
        mAreaVec.erase(mAreaVec.begin());
        aspectRatioVec.erase(aspectRatioVec.begin());
        circularityVec.erase(circularityVec.begin());
        squarenessVec.erase(squarenessVec.begin());

    }
    else
    {
        dataReady = false;
    }




    //diffInOut
    double avrInside  = 0;
    int countInside = 0;
    double avrOutside  = 0;
    for (int i = 0; i < mTargetFrame.rows; i++) {
        for (int j = 0; j < mTargetFrame.cols; j++) {
            if (mMask.at<uchar>(i, j) == 0) {
                avrOutside+=mTargetFrame.at<uchar>(i, j);
            }
            else
            {
                avrInside+=mTargetFrame.at<uchar>(i, j);
                countInside++;
            }
        }
    }
    int areaTotal = mTargetFrame.rows*mTargetFrame.cols;
    if(!countInside) diffInOut = 0;
    else if(countInside>=areaTotal)diffInOut = 0;
    else
    {
        avrInside/=countInside;
        avrOutside/=(areaTotal-countInside);
        double new_diffInOut = avrInside-avrOutside;
        diffInOut += (new_diffInOut-diffInOut)/5.0;
    }
    //if(diffInOut<60)dataReady = false;
    if (dataReady)
    {
        Scalar m, s;
        //area
//        meanStdDev(mAreaVec, m, s);
//        areaVar = s[0] / m[0];
        //aspect ratio
        meanStdDev(aspectRatioVec, m, s);
        aspectRatioVar = s[0] / m[0];
        aspectRatioMean = m[0];
        //circulartity
        meanStdDev(circularityVec, m, s);
        circularityVar = s[0] / m[0];
        circularityMean = m[0];
        //squareness
        meanStdDev(squarenessVec, m, s);
        squarenessVar = s[0] / m[0];
        squarenessMean = m[0];
        meanStdDev(frameDiffVec, m, s);
        frameDiffVar = s[0] / m[0];
        frameDiffMean = m[0];

    }
}

void Feature::calcTexture(int levels, int dx, int dy)
{

    assert(levels >= 2 && levels <= 256 && (levels & (levels - 1)) == 0);
    assert(dx >= 0 && dy >= 0 && dx + dy > 0);
    
    Mat temp;
    mTargetFrame.copyTo(temp);
    // TODO: implement my own version of 'equalizeHist' which accepts mask as an argument
    double minVal;
    minMaxLoc(temp, &minVal, NULL, NULL, NULL, mMask);
    uchar min = cvRound(minVal);
    for (int i = 0; i < temp.rows; i++) {
        for (int j = 0; j < temp.cols; j++) {
            if (mMask.at<uchar>(i, j) == 0) {
                temp.at<uchar>(i, j) = min;
            }
        }
    }
    equalizeHist(temp, temp);

#ifdef DEBUG_OUTPUT
    imshow("hist", temp);
#endif
    
    for (int i = 0; i < temp.rows; i++) {
        for (int j = 0; j < temp.cols; j++) {
            if (mMask.at<uchar>(i, j) == 255) {
                temp.at<uchar>(i, j) /= 256 / levels;
            }
        }
    }
    
    Mat glcm = Mat::zeros(Size(levels, levels), CV_64FC1);
    for (int i = 0; i < temp.rows; i++) {
        for (int j = 0; j < temp.cols; j++) {
            if (mMask.at<uchar>(i, j) == 255) {
                uchar l = temp.at<uchar>(i, j);
                int x1 = j + dx, y1 = i + dy;
                if (x1 < temp.cols && y1 < temp.rows && mMask.at<uchar>(y1, x1) == 255) {
                    uchar m = temp.at<uchar>(y1, x1);
                    glcm.at<double>(l, m) += 1;
                }
                int x2 = j - dx, y2 = i - dy;
                if (x2 >= 0 && y2 >= 0 && mMask.at<uchar>(y2, x2) == 255) {
                    uchar m = temp.at<uchar>(y2, x2);
                    glcm.at<double>(l, m) += 1;
                }
            }
        }
    }
    
    double sum = cv::sum(glcm)[0];
    if (sum == 0) {
        memset(texture, 0, sizeof(texture));
        return;
    }
    
    glcm *= 1.0 / sum;
    
    // in fact, the third one is not contrast...
    double entropy = 0, energy = 0, contrast = 0, homogenity = 0;
    for (int i = 0; i < levels; i++) {
        for (int j = 0; j < levels; j++) {
            double gij = glcm.at<double>(i, j);
            if(gij > 0) {
                entropy -= gij * log10(gij);
            }
            energy += gij * gij;
            contrast += (i - j) * (i - j) * gij;
            homogenity += 1.0 / (1 + (i - j) * (i - j)) * gij;
        }
    }
    
    texture[0] = entropy;
    texture[1] = energy;
    texture[2] = contrast;
    texture[3] = homogenity;

}

void Feature::calcFrequency()
{
    // TODO: optimize this part
    
    if (!dataReady) {
        return;
    }
    
    // limit n to integer power of 2 for simplicity
    // in fact, you can use function 'getOptimalDFTSize' to pad the input array
    assert((MAX_AREA_VEC_SIZE & (MAX_AREA_VEC_SIZE - 1)) == 0);
    
    vector<double> spec(MAX_AREA_VEC_SIZE);
    dft(mAreaVec, spec);
    
    double maxAmpl = 0;
    int idx = 0;
    for (int i = 1; i < MAX_AREA_VEC_SIZE; i += 2) {
        double ampl = (i == MAX_AREA_VEC_SIZE - 1) ? spec[i] :
            sqrt(spec[i] * spec[i] + spec[i + 1] * spec[i + 1]);
        if (ampl > maxAmpl) {
            maxAmpl = ampl;
            idx = (i + 1) / 2;
        }
    }
    if(idx!=1)
    {
         idx=idx;
    }
    //double fps = videoHandler->getVideoFPS();
    frequency = 30.0 / MAX_AREA_VEC_SIZE * idx;

#ifdef DEBUG_OUTPUT
    cout << "fps: " << fps << ", frequency: " << frequency << endl;
#endif
}

void Feature::calcDynamicFeatures()
{


}

Feature::Feature()
{
    circularityVar=0;
    squarenessVar=0;
    aspectRatioMean=0;
    roughness=0;
    diffInOut=0;
    frequency=0;
    areaVar=0;
}

void Feature::calc(const Region& region, const Mat& frame)
{
    Mat mNewFrame = frame(region.rect);
    //const Mat& mask = videoHandler->getDetector().getExtractor().getMask();

    if (videoHandler->mVideoChannel == 2)
    {
        const Mat& mask = m_worker2->getDetector().getExtractor().getMask();
        mMask = mask(region.rect);
    }
    else if (videoHandler->mVideoChannel == 3)
    {
        const Mat& mask = m_worker3->getDetector().getExtractor().getMask();
        mMask = mask(region.rect);
    }
    else if (videoHandler->mVideoChannel == 1)
    {
        const Mat& mask = m_worker->getDetector().getExtractor().getMask();
        mMask = mask(region.rect);
    }
    else  //(videoHandler->mVideoChannel == 0) // default - for Training mode
    {
        const Mat& mask = videoHandler->getDetector().getExtractor().getMask();
        mMask = mask(region.rect);

    }


#ifndef MODE_GRAYSCALE
    cvtColor(mROI, mGray, CV_BGR2GRAY);
#else
    if(mTargetFrame.rows)
    {
        Rect roiTar ,roiNew;

        roiTar.height = min(mTargetFrame.rows,mNewFrame.rows);
        roiTar.width = min(mTargetFrame.cols,mNewFrame.cols);
        roiTar.x = 0;//(mTargetFrame.cols-roiTar.width)/2;
        roiTar.y = 0;//(mTargetFrame.rows-roiTar.height)/2;
        roiNew.height = roiTar.height ;
        roiNew.width = roiTar.width ;
        roiNew.x = 0;//(mNewFrame.cols-roiNew.width)/2;
        roiNew.y = 0;//(mNewFrame.rows-roiNew.height)/2;
        double frameDiff;
        Mat diff ;
        absdiff(mTargetFrame(roiTar),mNewFrame(roiNew),diff);
        frameDiff  = sum(diff)[0];
        /*
         * double newFrameDiff =   99999998;
        //minimize frameDiff
        while(newFrameDiff!=frameDiff){
            frameDiff = newFrameDiff;

            double tempDiff  ;
            enum directio{up,down,left,right,center} direction;
            //center
            absdiff(mTargetFrame(roiTar),mNewFrame(roiNew),diff);
            tempDiff  = sum(diff)[0];
            if(tempDiff<newFrameDiff)
            {
                newFrameDiff = tempDiff;
                direction = center;
            }
            continue;
            //down
            if((roiTar.height+roiTar.y)<mTargetFrame.rows)
            {
                roiTar.y++;
                absdiff(mTargetFrame(roiTar),mNewFrame(roiNew),diff);
                tempDiff  = sum(diff)[0];
                if(tempDiff<newFrameDiff)
                {
                    newFrameDiff = tempDiff;
                    direction = down;
                }
                roiTar.y--;
            }
            //up
            if((roiTar.y))
            {
                roiTar.y--;
                absdiff(mTargetFrame(roiTar),mNewFrame(roiNew),diff);
                tempDiff  = sum(diff)[0];
                if(tempDiff<newFrameDiff)
                {
                    newFrameDiff = tempDiff;
                    direction = up;
                }
                roiTar.y++;
            }
            //right
            if((roiTar.width+roiTar.x)<mTargetFrame.cols)
            {
                roiTar.x++;
                absdiff(mTargetFrame(roiTar),mNewFrame(roiNew),diff);
                tempDiff  = sum(diff)[0];
                if(tempDiff<newFrameDiff)
                {
                    newFrameDiff = tempDiff;
                    direction = right;
                }
                roiTar.x--;
            }
            //left
            if(roiTar.x)
            {
                roiTar.x--;
                absdiff(mTargetFrame(roiTar),mNewFrame(roiNew),diff);
                tempDiff  = sum(diff)[0];
                if(tempDiff<newFrameDiff)
                {
                    newFrameDiff = tempDiff;
                    direction = left;
                }
                roiTar.x++;
            }

            switch(direction)
            {
            case left:
                roiTar.x--;
                break;
            case right:
                roiTar.x++;
                break;
            case up:
                roiTar.y--;
                break;
            case down:
                roiTar.y++;
                break;
            case center:
                break;
            }
        }
        */
        frameDiffVec.push_back(frameDiff/(roiTar.height*roiTar.width));
        if(frameDiffVec.size()>MAX_AREA_VEC_SIZE)
        {
            frameDiffVec.erase(frameDiffVec.begin());
        }
    }

    mTargetFrame = mNewFrame;
#endif
    calcColorFeature();
    calcGeometryFeature(region);
    calcDynamicFeatures();
    calcTexture();
    //calcFrequency();

}

void Feature::merge(const vector<const Feature*>& src, Feature& feature)
{
    vector<double>::size_type maxAreaVecSize = 0;
    for (vector<const Feature*>::const_iterator it = src.begin(); it != src.end(); it++) {
        vector<double>::size_type areaVecSize = (*it)->mAreaVec.size();
        if (areaVecSize > maxAreaVecSize) {
            maxAreaVecSize = areaVecSize;
        }
    }
    
    vector<double>(maxAreaVecSize, 0).swap(feature.mAreaVec);
    
    for (vector<const Feature*>::const_iterator it1 = src.begin(); it1 != src.end(); it1++) {
        const vector<double>& areaVec = (*it1)->mAreaVec;
        vector<double>::reverse_iterator it2 = feature.mAreaVec.rbegin();

        for (vector<double>::const_reverse_iterator it4 = areaVec.rbegin(); it4 != areaVec.rend(); it4++) {
            *(it2++) += *it4;
        }
    }
}

void Feature::printValue() const
{
    cout   << circularityMean<<" circularityMean\n "
           << squarenessMean<<" squarenessMean\n "
           << aspectRatioMean<<" aspectRatioMean\n "
           << frameDiffMean<< " frameDiffMean\n"
           << circularityVar<<" circularityVar\n "
           << squarenessVar<<" squarenessVar\n "
           << aspectRatioVar<<" aspectRatioVar\n "
           << frameDiffVar<<" frameDiffVar\n "
           << roughness<<" roughness\n "
           << diffInOut<<" diffInOut\n "
           << texture[0]<<" texture\n "
           << texture[1]<<" texture\n "
           << texture[2]<<" texture\n "
           << texture[3]<<" texture\n";
}

bool Feature::checkValid()
{
    if(diffInOut< mConfig._config.diffInOut||frameDiffMean<6.0)return false;
    if(!circularityMean
            *squarenessMean
            *aspectRatioMean
            *frameDiffMean
            *circularityVar
            *squarenessVar*aspectRatioVar*roughness*diffInOut*texture[0]*texture[1]*texture[2]*texture[3])
        return false;
        return true;
}

Feature::operator Mat() const
{
    return (Mat_<float>(1, LEN) <<
//            red[0], red[1], red[2], red[3],
//            gray[0], gray[1], gray[2], gray[3],
//            saturation[0], saturation[1], saturation[2], saturation[3],
              circularityMean
            , squarenessMean
            , aspectRatioMean
            , frameDiffMean
            , circularityVar
            , squarenessVar
            , aspectRatioVar
            , roughness
            , diffInOut
            , texture[0]
            , texture[1]
            , texture[2]
            , texture[3]
            );
}

ifstream& operator>>(ifstream& ifs, Feature& feature)
{
    ifs /*>> feature.red[0] >> feature.red[1]
        >> feature.red[2] >> feature.red[3]
        >> feature.gray[0] >> feature.gray[1]
        >> feature.gray[2] >> feature.gray[3]
        >> feature.saturation[0] >> feature.saturation[1]
        >> feature.saturation[2] >> feature.saturation[3]*/
         >>feature.circularityMean
         >>feature.squarenessMean
         >>feature.aspectRatioMean
         >>feature.frameDiffMean
         >>feature.circularityVar
         >>feature.squarenessVar
         >>feature.aspectRatioVar
         >>feature.roughness
         >>feature.diffInOut
         >>feature.texture[0]
         >>feature.texture[1]
         >>feature.texture[2]
         >>feature.texture[3];
    return ifs;
}

ofstream& operator<<(ofstream& ofs, const Feature& feature)
{
    ofs /*<< feature.red[0] << " " << feature.red[1] << " "
        << feature.red[2] << " " << feature.red[3] << " "
        << feature.gray[0] << " " << feature.gray[1] << " "
        << feature.gray[2] << " " << feature.gray[3] << " "
        << feature.saturation[0] << " " << feature.saturation[1] << " "
        << feature.saturation[2] << " " << feature.saturation[3] << " "*/
            <<feature.circularityMean <<" "
            <<feature.squarenessMean  <<" "
            <<feature.aspectRatioMean  <<" "
            <<feature.frameDiffMean    <<" "
            <<feature.circularityVar   <<" "
            <<feature.squarenessVar    <<" "
            <<feature.aspectRatioVar   <<" "
            <<feature.roughness        <<" "
            <<feature.diffInOut        <<" "
            <<feature.texture[0]       <<" "
            <<feature.texture[1]       <<" "
            <<feature.texture[2]       <<" "
            <<feature.texture[3]       <<" ";
    return ofs;
}

#ifdef PHUONGS_ALGORITHM
template <typename T>
cv::Mat plotGraph(const std::vector<T>& vals, int YRange[2],int xstep = 1)
{

    auto it = minmax_element(vals.begin(), vals.end());
    float scale = 1./ceil(*it.second - *it.first);
    float bias = *it.first;
    int rows = YRange[1] - YRange[0] + 1;
    cv::Mat image = Mat::zeros( rows, vals.size()*xstep, CV_8UC3 );
    image.setTo(0);
    for (int i = 0; i < (int)vals.size()-1; i+=1)
    {
        int x = i*xstep;
        cv::line(image, cv::Point(x, rows - 1 - (vals[i] - bias)*scale*YRange[1]), cv::Point(x+xstep, rows - 1 - (vals[i+1] - bias)*scale*YRange[1]), Scalar(255, 0, 0), 1);
    }

    return image;
}

void Feature::printAreaVec() const
{
    int range[2] = {-200,200};

    imshow("mAreaVec",plotGraph(mAreaVec,range,10));
      imshow("frameDiffVec",plotGraph(frameDiffVec,range,10));


    std::vector<double> vec;//(p,dftmat.size());
    dft(mAreaVec,vec);
    imshow("spec",plotGraph(vec,range,10 ));
    std::vector<double> ampl(vec.size());
    for (int i = 1; i < MAX_AREA_VEC_SIZE; i += 2) {
        ampl[i] = (i == MAX_AREA_VEC_SIZE - 1) ? vec[i] :
            sqrt(vec[i] * vec[i] + vec[i + 1] * vec[i + 1]);
//        if (ampl > maxAmpl) {
//            maxAmpl = ampl;
//            idx = (i + 1) / 2;
//        }
    }
    imshow("ampl",plotGraph(ampl,range,20));
    //cv::plot::createPlot2d (mAreaVec);

//    vector<double>::size_type size = mAreaVec.size();
    
//    for (int i = 0; i < size; i++) {
//        cout << mAreaVec[i];
//        if (i != size - 1) {
//            cout << ", ";
//        } else {
//            cout << endl;
//        }
//    }
}
#endif

/**************** FeatureAnalyzer ****************/

void FeatureAnalyzer::featureMerge(Target& target, const map<int, Target>& targets, const vector<int>& keys)
{
    vector<const Feature*> featureVec;
    for (vector<int>::const_iterator it = keys.begin(); it != keys.end(); it++) {
        // const map can't be accessed by operator '[]', so use function 'find' instead
        map<int, Target>::const_iterator iter = targets.find(*it);
        featureVec.push_back(&(iter->second.feature));
    }
    Feature::merge(featureVec, target.feature);
}

void FeatureAnalyzer::targetUpdate(map<int, Target>& targets)
{
    for (map<int, Target>::iterator it = targets.begin(); it != targets.end(); )
    {
        Target& target = it->second;
        
        if (target.type == Target::TARGET_LOST)
        {
            int maxTimes = min(target.times * 2, 10);
            if (target.lostTimes >= maxTimes)
            {
                targets.erase(it++);
                continue;
            }
        }
        else
        {
            if (target.lostTimes != 0) {
                target.lostTimes = 0;
            }
            if (target.type == Target::TARGET_MERGED)
            {
                vector<int>& keys = target.mergeSrc;
                featureMerge(target, targets, keys);
                for (vector<int>::const_iterator it2 = keys.begin(); it2 != keys.end(); it2++) {
                    targets.erase(targets.find(*it2));
                }
                vector<int>().swap(keys);
            }
        }
        it++;
    }
    
    for (map<int, Target>::iterator it = targets.begin(); it != targets.end(); it++) {
        Target& target = it->second;
        if (target.type != Target::TARGET_LOST) {
            target.feature.calc(target.region, mFrame);
        }
    }
}

void FeatureAnalyzer::analyze(const Mat& frame, map<int, Target>& targets)
{
    mFrame = frame;
    
    targetUpdate(targets);
    

#ifdef DEBUG_MODE
    Mat temp;
    mFrame.copyTo(temp);
    for (map<int, Target>::iterator it = targets.begin(); it != targets.end(); it++) {
        rectangle(temp, it->second.region.rect, Scalar(0, 255, 0));
    }
    namedWindow("frame");
    moveWindow("frame", 10, 500);
    imshow("frame", temp);
#endif
}
