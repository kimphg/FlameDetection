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

extern VideoHandler* videoHandler;

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
    avrInside/=countInside;
    avrOutside/=(mTargetFrame.rows*mTargetFrame.cols-countInside);
    double new_diffInOut = avrInside-avrOutside;
    diffInOut += (new_diffInOut-diffInOut)/5.0;
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
    mROI = frame(region.rect);
#ifndef MODE_GRAYSCALE
    cvtColor(mROI, mGray, CV_BGR2GRAY);
#else
    mTargetFrame = mROI;
#endif
    const Mat& mask = videoHandler->getDetector().getExtractor().getMask();
    mMask = mask(region.rect);
    


    calcColorFeature();
    calcGeometryFeature(region);
    calcDynamicFeatures();
    calcTexture();
    calcFrequency();

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
           << circularityVar<<" circularityVar\n "
           << squarenessVar<<" squarenessVar\n "
           << aspectRatioVar<<" aspectRatioVar\n "
           << roughness<<" roughness\n "
           << frequency<<" frequency\n "
           << diffInOut<<" diffInOut\n "
           << texture[0]<<" texture\n "
           << texture[1]<<" texture\n "
           << texture[2]<<" texture\n "
           << texture[3]<<" texture\n";
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
            , circularityVar
            , squarenessVar
            , aspectRatioVar
            , roughness
            , frequency
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
        >> feature.circularityMean
        >> feature.squarenessMean
        >> feature.aspectRatioMean
        >> feature.circularityVar
        >> feature.squarenessVar
        >> feature.aspectRatioVar
        >> feature.roughness
        >> feature.frequency
        >> feature.diffInOut
        >> feature.texture[0] >> feature.texture[1]
        >> feature.texture[2] >> feature.texture[3];
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
            << feature.circularityMean  <<" "
            << feature.squarenessMean   <<" "
            << feature.aspectRatioMean  <<" "
            << feature.circularityVar   <<" "
            << feature.squarenessVar    <<" "
            << feature.aspectRatioVar   <<" "
            << feature.roughness        <<" "
            << feature.frequency          <<" "
            << feature.diffInOut        <<" "
            << feature.texture[0] <<" "
            << feature.texture[1] <<" "
            << feature.texture[2] <<" "
            << feature.texture[3] <<" ";
    return ofs;
}
/*
cirMean  squarMea RatioMn circVar  sqrnesVar aRatioVar roughne  areaVar   diInOut texture[0] texture[1] texture[2] texture[3]
0.415094 0.573003 1.56755 0.145607 0.0280377 0.0590974 0.795831 0.0543733 73.5503 1.84814 0.0404534 9.03426 0.438961 1
0.434756 0.580289 1.59733 0.150543 0.0341893 0.0526833 0.844623 0.0631722 73.1706 1.95472 0.0256731 8.99546 0.417328 1
0.350158 0.529605 1.74341 0.15759  0.0752709 0.0902018 0.803676 0.0637267 72.0421 1.86045 0.0336926 22.9309 0.301224 1
0.494971 0.743801 2.84612 0.278997 0.0596443 0.336257  0.784207 0.306249  68.5016 2.15432 0.0112956 11.392 0.400044 0
0.21536  0.55559  3.52414 0.164796 0.0713414 0.0720209 0.699721 0.135722  72.0908 2.2035 0.00732847 29.9016 0.190379 0
0.371405 0.643159 2.18033 0.189342 0.153243  0.364404  0.745647 0.399162  91.9023 2.02802 0.013722 23.5682 0.268503 0
0.416399 0.62339  2.50821 0.148906 0.0332985 0.192598  0.807184 0.250325  61.1766 2.029 0.0175491 24.6659 0.307932 0
0.518535 0.606294 1.29253 0.073588 0.039512  0.0741743 0.848846 0.0742539 70.5952 2.1599 0.00831926 29.4977 0.189994 0
0.351387 0.594082 1.86326 0.080379 0.0173382 0.0314293 0.776051 0.0409589 70.0365 2.30467 0.00606879 30.6101 0.22897 0
0.292286 0.664877 2.18902 0.230523 0.0694317 0.355992  0.81842  0.365814  32.1312 2.25004 0.00745601 18.3084 0.287047 0
0.055918 0.381996 11.3989 0.067457 0.0659442 0.0167595 0.761466 0.0776157 98.4974 2.27298 0.00724209 28.2329 0.264406 0
0.228261 0.611139 6.03836 0.099879 0.0484991 0.0271229 0.920874 0.0525302 46.015  2.24197 0.00704156 34.6036 0.186759 0
0.055985 0.371824 11.477  0.052998 0.0601627 0.0264246 0.782176 0.0770014 99.478  2.29632 0.00741294 31.219 0.252166 0
0.255382 0.756727 4.0134  0.435255 0.0593902 0.31255   0.716844 0.306973  47.3558 2.27197 0.00701942 26.8224 0.22487 0
0.20487  0.742155 4.68393 0.359333 0.0495042 0.244072  0.686498 0.221085  46.6606 2.29737 0.00645875 25.3859 0.230093 0
0.167798 0.730653 5.23636 0.207808 0.0301134 0.188933  0.655108 0.15467   46.5731 2.26506 0.00817991 28.2112 0.258099 0
0.223982 0.678741 7.56119 0.051692 0.0253913 0.0426294 0.92368  0.0515003 45.5755 2.21277 0.00776789 31.1698 0.230452 0
0.144683 0.709961 5.70865 0.283938 0.0555304 0.141081  0.57214  0.104636  46.3692 2.25991 0.00716266 28.2086 0.24206 0
0.223292 0.67291  7.52355 0.044961 0.0232181 0.048476  0.931409 0.0601913 49.1084 2.26299 0.00677054 34.3874 0.212796 0
0.214137 0.786111 9.47987 0.250147 0.0204964 0.233355  0.953267 0.177479  37.1381 2.24102 0.00748833 34.302 0.222142 0
*/
#ifdef PHUONGS_ALGORITHM
void Feature::printAreaVec() const
{
    vector<double>::size_type size = mAreaVec.size();
    
    for (int i = 0; i < size; i++) {
        cout << mAreaVec[i];
        if (i != size - 1) {
            cout << ", ";
        } else {
            cout << endl;
        }
    }
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
    
    Mat temp;
    mFrame.copyTo(temp);
    for (map<int, Target>::iterator it = targets.begin(); it != targets.end(); it++) {
        rectangle(temp, it->second.region.rect, Scalar(0, 255, 0));
    }
#ifdef DEBUG_MODE
    namedWindow("frame");
    moveWindow("frame", 10, 500);
    imshow("frame", temp);
#endif
}
