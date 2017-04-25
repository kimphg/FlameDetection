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
    double new_aspectRatio = 0;
    double new_roughness = 0;
    
    const vector<ContourInfo*>& contours = region.contours;
    for (vector<ContourInfo*>::const_iterator it = contours.begin(); it != contours.end(); it++) {
        const vector<Point>& contour = (*it)->contour;
        double area = (*it)->area;
        
        double perimeter = arcLength(contour, true);
        RotatedRect minRect = minAreaRect(Mat(contour));
        vector<Point> hull;
        convexHull(contour, hull);
        double perimeterHull = arcLength(hull, true);
        double width = minRect.size.width, height = minRect.size.height;
        
        new_circularity += area * (4 * 3.1416 * area / (perimeter * perimeter));
        new_squareness  += area * (area / (width * height));
        new_aspectRatio += area * (1.0 * min(width, height) / max(width, height));
        new_roughness   += area * (perimeterHull / perimeter);
    }
    
    new_circularity /= mArea;
    new_squareness  /= mArea;
    new_aspectRatio /= mArea;
    new_roughness   /= mArea;
    circularity +=(new_circularity-circularity)/10;
    squareness +=(new_squareness-squareness)/10;
    aspectRatio +=(new_aspectRatio-aspectRatio)/10;
    roughness +=(new_roughness-roughness)/10;
}

void Feature::calcTexture(int levels, int dx, int dy)
{

    assert(levels >= 2 && levels <= 256 && (levels & (levels - 1)) == 0);
    assert(dx >= 0 && dy >= 0 && dx + dy > 0);
    
    Mat temp;
    mGray.copyTo(temp);
#ifdef PHUONGS_ALGORITHM
    double avrInside  = 0;
    int countInside = 0;
    double avrOutside  = 0;
    for (int i = 0; i < temp.rows; i++) {
        for (int j = 0; j < temp.cols; j++) {
            if (mMask.at<uchar>(i, j) == 0) {
                avrOutside+=temp.at<uchar>(i, j);
            }
            else
            {
                avrInside+=temp.at<uchar>(i, j);
                countInside++;
            }
        }
    }
    avrInside/=countInside;
    avrOutside/=(temp.rows*temp.cols-countInside);
    double new_diffInOut = avrInside-avrOutside;
    diffInOut += (new_diffInOut-diffInOut)/10;

#endif
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
    
    if (mAreaVec.size() < MAX_AREA_VEC_SIZE) {
        frequency = -1;
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
    
    double fps = videoHandler->getVideoFPS();
    frequency = fps / MAX_AREA_VEC_SIZE * idx;

#ifdef DEBUG_OUTPUT
    cout << "fps: " << fps << ", frequency: " << frequency << endl;
#endif
}

void Feature::calcAreaVar()
{
    // TODO: optimize this part
    
    if (mAreaVec.size() < MAX_AREA_VEC_SIZE) {
        areaVar = -1;
        //areaMeanStdDev = -1;
        return;
    }
    
    Scalar m, s;
    meanStdDev(mAreaVec, m, s);
    areaVar = s[0] / m[0];
    //areaMeanStdDev = s[0];
#ifdef DEBUG_OUTPUT
    cout << "areaVar: " << areaVar << endl;
#endif
}

void Feature::calc(const Region& region, const Mat& frame)
{
    mROI = frame(region.rect);
#ifndef MODE_GRAYSCALE
    cvtColor(mROI, mGray, CV_BGR2GRAY);
#else
    mGray = mROI;
#endif
    const Mat& mask = videoHandler->getDetector().getExtractor().getMask();
    mMask = mask(region.rect);
    mArea = 0;
    
    const vector<ContourInfo*>& contours = region.contours;
    for (vector<ContourInfo*>::const_iterator it = contours.begin(); it != contours.end(); it++) {
        mArea += (*it)->area;
    }

    calcColorFeature();
    calcGeometryFeature(region);
    
    if (mAreaVec.size() >= MAX_AREA_VEC_SIZE) {
        mAreaVec.erase(mAreaVec.begin());

        calcTexture();
        ready = true;
    }
    else
    {
        ready = false;
    }
    mAreaVec.push_back(mArea);
    
    //calcFrequency();
    calcAreaVar();
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

Feature::operator Mat() const
{
    return (Mat_<float>(1, LEN) <<
//            red[0], red[1], red[2], red[3],
//            gray[0], gray[1], gray[2], gray[3],
//            saturation[0], saturation[1], saturation[2], saturation[3],
            circularity, squareness, aspectRatio, roughness,areaVar,diffInOut,
            texture[0], texture[1], texture[2], texture[3]);
}
/*
        0.0684441   0.352894    0.269461 0.539347   665.499 44.5855     2.27866 0.00668438 21.6364 0.261937 0
        0.17747     0.49296     0.791045 0.617529   370.907 25.1582     2.28684 0.00617608 20.8837 0.251204 0
        0.0728507   0.389019    0.22619  0.559712   617.373 48.4005     2.25236 0.00671369 23.6601 0.250367 0
        0.151407    0.561457    0.761194 0.575805   457.873 25.0511     2.22639 0.00729021 14.6032 0.301147 0
        0.12807     0.389289    0.296875 0.655472   578.655 41.1841     2.29214 0.00609746 25.3556 0.258412 0
        0.136834    0.442853    0.838235 0.581234   466.788 28.5133     2.23118 0.00716583 13.3224 0.303049 0




        0.153457 0.55308    0.74701     0.515677    99.8568 75.127      2.12291 0.0128255 10.148  0.379672 1
        0.183388 0.506102   0.802381    0.569849    99.5972 73.6027     2.11495 0.013334  8.7505  0.387453 1
        0.208758 0.499534   0.881603    0.60266     86.6017 71.8819     2.09712 0.0134587 8.68596 0.394787 1
        0.218334 0.545658   0.846196    0.606333    81.2897 72.1463     2.12898 0.0108633 9.62159 0.385817 1
        0.254011 0.555027   0.736264    0.627839    75.7726 63.6343     2.17223 0.0093245 10.4298 0.353623 1
        0.184303 0.537089   0.700842    0.545385    74.4444 71.0725     2.14625 0.0116139 10.8292 0.360753 1
*/
ifstream& operator>>(ifstream& ifs, Feature& feature)
{
    ifs /*>> feature.red[0] >> feature.red[1]
        >> feature.red[2] >> feature.red[3]
        >> feature.gray[0] >> feature.gray[1]
        >> feature.gray[2] >> feature.gray[3]
        >> feature.saturation[0] >> feature.saturation[1]
        >> feature.saturation[2] >> feature.saturation[3]*/
        >> feature.circularity >> feature.squareness
        >> feature.aspectRatio >> feature.roughness
        >> feature.areaVar   >>feature.diffInOut
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
        << feature.circularity << " " << feature.squareness << " "
        << feature.aspectRatio << " " << feature.roughness << " "
        << feature.areaVar   << " " <<feature.diffInOut  << " "
        << feature.texture[0] << " " << feature.texture[1] << " "
        << feature.texture[2] << " " << feature.texture[3] << " ";
    return ofs;
}

#ifdef DEBUG_OUTPUT
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
