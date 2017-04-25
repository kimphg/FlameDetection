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

Feature::Feature()
{
    circularity=0;
    squareness=0;
    aspectRatio=0;
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
0.225679 0.621564 0.683939 0.595304 0.0426069 10.6425 2.23783 0.00767549 15.8705 0.319143 0
0.212579 0.610313 0.657886 0.588981 0.0436852 18.6286 2.24482 0.00749072 16.4075 0.314524 0

0.11348 0.529981 0.367314 0.526054 0.145431 44.0843 2.26172 0.00677292 19.1694 0.260525 0
0.110111 0.512849 0.326862 0.540649 0.143896 48.0901 2.3017 0.0060758 20.6976 0.258765 0
0.268385 0.512409 0.625367 0.669439 0.222379 2.15528 2.22599 0.00752643 20.5399 0.280366 0

0.109066 0.364066 0.337469 0.617353 0.230319 39.8793 2.31342 0.00573753 27.7838 0.236201 0
0.111655 0.432216 0.76718 0.510079 0.306986 25.6209 2.30982 0.00597598 26.8203 0.266877 0
0.104498 0.349127 0.359052 0.605848 0.200261 38.9046 2.31457 0.0057017 27.4154 0.241347 0
0.0913013 0.388416 0.810138 0.487751 0.244727 29.4235 2.29637 0.0067215 25.0286 0.275339 0

0.0820308 0.368375 0.805901 0.480071    0.166681 31.1757 2.24107 0.00766098 14.8116 0.309657 0
0.111956 0.354679 0.304951 0.641406     0.13134 36.7685 2.29129 0.00621622 29.3734 0.238853 0
0.0968052 0.375595 0.823178 0.515083    0.143599 30.1015 2.2091 0.00767414 13.79 0.298751 0
0.115631 0.356416 0.302699 0.650743     0.143699 34.7197 2.33154 0.00541361 32.4832 0.19958 0
0.108057 0.398418 0.887319 0.534987     0.128549 28.936 2.17757 0.00850535 11.5944 0.333365 0
0.10553 0.346207 0.302615 0.629115      0.155707 32.8738 2.32783 0.00548159 30.1264 0.20797 0
0.168371 0.341471 0.941088 0.676576     0.104627 38.6931 2.16416 0.00968045 11.0009 0.336037 0
0.0732417 0.272483 0.428661 0.544335    0.115197 29.7413 2.35167 0.00498787 35.4028 0.197585 0
0.186151 0.376504 0.952085 0.677864     0.0705175 35.9811 2.17208 0.00888694 9.64094 0.372929 0
0.0721759 0.284799 0.391608 0.543084    0.120215 29.0296 2.33033 0.0053518 32.793 0.205224 0
0.207634 0.398882 0.944696 0.696231     0.063132 33.8767 2.14288 0.00940555 10.2148 0.346597 0

0.185884 0.55598 0.741256 0.55555       0.0535428 65.9839  2.13146 0.0125267 10.2167 0.40291 1
0.214109 0.556452 0.686131 0.597021     0.039062 70.2287 2.14779 0.00988294 9.25978 0.378577 1
0.216474 0.541828 0.696821 0.605822     0.0264021 73.992 2.10584 0.0148444 9.01753 0.394002 1
0.229498 0.533143 0.672069 0.625877     0.0355498 76.3203 2.14521 0.010975 9.65087 0.373557 1
0.270906 0.549181 0.626591 0.669713     0.0414438 76.7153 2.12957 0.0122567 9.99602 0.398517 1
0.276261 0.560143 0.630591 0.669773     0.0449739 75.8957 2.16714 0.0113331 10.939 0.383929 1
0.240264 0.555957 0.693162 0.622345     0.0458402 75.3957 2.13336 0.012241 11.0225 0.383023 1
0.23571 0.549984 0.653938 0.62548       0.0463237 77.1557 2.11554 0.0112645 9.33507 0.403924 1
0.246454 0.553963 0.583566 0.650469     0.03706 77.1586 2.13898 0.0113622 9.9155 0.37636 1
0.225535 0.548682 0.641151 0.618743     0.0274363 74.6745 2.14111 0.0104251 10.8478 0.344294 1
0.209729 0.544063 0.712305 0.591944 0.0298222 72.7906 2.12559 0.0123053 9.9033 0.366714 1
0.246269 0.556743 0.684674 0.628461 0.0312892 71.1051 2.14934 0.0120117 10.9821 0.378316 1
0.284551 0.569814 0.636551 0.66954 0.0303267 70.773 2.11735 0.012838 9.92028 0.396432 1
0.229601 0.557188 0.681085 0.605337 0.0297539 72.2021 2.12291 0.0128255 10.148 0.379672 1
0.203273 0.540991 0.719245 0.580493 0.0296397 73.6732 2.11495 0.013334 8.7505 0.387453 1
0.203172 0.522826 0.778743 0.587266 0.0256674 73.6477 2.09712 0.0134587 8.68596 0.394787 1
0.210892 0.533018 0.801738 0.597938 0.0241677 73.3626 2.12898 0.0108633 9.62159 0.385817 1
0.230279 0.545376 0.791285 0.613217 0.0225946 69.5605 2.17223 0.0093245 10.4298 0.353623 1
0.220817 0.544534 0.759591 0.596582 0.0220779 69.1659 2.14625 0.0116139 10.8292 0.360753 1
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
