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
        //new_aspectRatio += area * (1.0 * min(width, height) / max(width, height));
        new_roughness   += area * (perimeterHull / perimeter);
    }
    

    //circularity +=(new_circularity-circularity)/10;
    //squareness +=(new_squareness-squareness)/10;
    //aspectRatioMean +=(new_aspectRatio-aspectRatioMean)/10;
    roughness +=(new_roughness-roughness)/10;
}

void Feature::calcTexture(int levels, int dx, int dy)
{

    assert(levels >= 2 && levels <= 256 && (levels & (levels - 1)) == 0);
    assert(dx >= 0 && dy >= 0 && dx + dy > 0);
    
    Mat temp;
    mGray.copyTo(temp);
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

void Feature::calcDynamicFeatures()
{
    //diffInOut
    double avrInside  = 0;
    int countInside = 0;
    double avrOutside  = 0;
    for (int i = 0; i < mGray.rows; i++) {
        for (int j = 0; j < mGray.cols; j++) {
            if (mMask.at<uchar>(i, j) == 0) {
                avrOutside+=mGray.at<uchar>(i, j);
            }
            else
            {
                avrInside+=mGray.at<uchar>(i, j);
                countInside++;
            }
        }
    }
    avrInside/=countInside;
    avrOutside/=(mGray.rows*mGray.cols-countInside);
    double new_diffInOut = avrInside-avrOutside;
    diffInOut += (new_diffInOut-diffInOut)/10.0;
    //area
    if (mAreaVec.size() < MAX_AREA_VEC_SIZE) {
        dataReady = false;
    }
    else
    {
        Scalar m, s;
        meanStdDev(mAreaVec, m, s);
        areaVar = s[0] / m[0];
    }
    //
    if(aspectRatioVec.size()>=MAX_AREA_VEC_SIZE)
    {
        Scalar m, s;
        meanStdDev(aspectRatioVec, m, s);
        aspectRatioVar = s[0] / m[0];
        aspectRatioMean = m[0];
    }
    else
    {
        dataReady = false;
    }

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
        dataReady = true;
        calcDynamicFeatures();
        calcTexture();
        mAreaVec.erase(mAreaVec.begin());
        aspectRatioVec.erase(aspectRatioVec.begin());
    }
    else
    {
        dataReady = false;
    }

    //
    double aspectRatio = ((double)region.rect.width)/region.rect.height;
    if(aspectRatio<1)aspectRatio = 1.0/aspectRatio;
    assert(aspectRatio>=1);
    aspectRatioVec.push_back(aspectRatio);
    mAreaVec.push_back(mArea);
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

Feature::operator Mat() const
{
    return (Mat_<float>(1, LEN) <<
//            red[0], red[1], red[2], red[3],
//            gray[0], gray[1], gray[2], gray[3],
//            saturation[0], saturation[1], saturation[2], saturation[3],
            circularityVar, squarenessVar, aspectRatioMean, aspectRatioVar,
            roughness, areaVar, diffInOut,
            texture[0], texture[1], texture[2], texture[3]);
}

ifstream& operator>>(ifstream& ifs, Feature& feature)
{
    ifs /*>> feature.red[0] >> feature.red[1]
        >> feature.red[2] >> feature.red[3]
        >> feature.gray[0] >> feature.gray[1]
        >> feature.gray[2] >> feature.gray[3]
        >> feature.saturation[0] >> feature.saturation[1]
        >> feature.saturation[2] >> feature.saturation[3]*/
        >> feature.circularityVar >> feature.circularityVar
        >> feature.aspectRatioMean >> feature.aspectRatioVar>> feature.roughness
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
        << feature.circularityVar << " " << feature.squarenessVar << " "
        << feature.aspectRatioMean << " " << feature.aspectRatioVar << " "
        <<feature.roughness << " "
        << feature.areaVar   << " " <<feature.diffInOut  << " "
        << feature.texture[0] << " " << feature.texture[1] << " "
        << feature.texture[2] << " " << feature.texture[3] << " ";
    return ofs;
}
/*
0.376236 0.579736 0.513485 0.0910183 0.813084 0.305358 73.688 2.22974 0.00704165 31.5547 0.218125 0
0.146479 0.388543 1.11473  0.182059  0.579836 0.0988645 27.8445 2.10784 0.00952845 32.3658 0.250229 0
0.371029 0.577988 0.507817 0.0931047 0.813959 0.274086 74.4943 2.18191 0.00808991 30.7078 0.225462 0
0.189852 0.41968  1.14693  0.161657  0.619627 0.131024 28.0261 1.99812 0.0122217 28.8341 0.271016 0
0.350039 0.564893 0.502369 0.0950097 0.799442 0.21847 74.1376 2.16718 0.00834711 28.9864 0.233482 0
0.272545 0.479264 1.21994  0.117834  0.686729 0.145392 27.1445 1.85162 0.0223736 30.9083 0.274067 0
0.340137 0.558099 0.488392 0.0365847 0.789955 0.145111 73.1101 2.13342 0.00878906 40.9821 0.20408 0
0.304672 0.565968 0.371253 0.291883  0.878537 0.364841 2.77255 1.87405 0.0149382 46.0678 0.146106 0
0.421152 0.610712 0.601029 0.0886085 0.822567 0.0836591 42.6385 2.17346 0.0126956 14.2288 0.327626 1
0.392236 0.605773 0.62375  0.100317 0.799903  0.0575647 41.2139 2.08572 0.0135286 12.7289 0.339164 1
0.36889  0.595184 0.64408  0.115661 0.786545  0.0552047 41.1443 2.16481 0.0109562 11.6146 0.353305 1
0.338972 0.580727 0.66354  0.122792 0.765669  0.0536064 43.2742 2.16091 0.0107671 11.088 0.36958 1
0.355878 0.596067 0.691116 0.11311 0.772909   0.0483589 45.8656 2.08963 0.0153151 12.8132 0.399619 1
0.376996 0.601943 0.713038 0.0933031 0.793195 0.036363  48.5858 2.12739 0.0104317 14.3288 0.357296 1
0.36307  0.599245 0.728717 0.0688098 0.787554 0.039321  51.0018 2.13176 0.0130141 12.9171 0.336676 1
0.390317 0.608434 0.722001 0.0663886 0.803991 0.0484484 50.6438 2.06871 0.0131221 13.93 0.345771 1
0.414343 0.619932 0.711253 0.0780828 0.815157 0.0512223 50.9703 2.13184 0.0148702 14.0961 0.350149 1
0.407949 0.627968 0.68979  0.0929591 0.80643 0.0510396  51.2765 2.04448 0.0144487 15.1534 0.36553 1
0.387897 0.628299 0.669363 0.0980457 0.79047 0.05528    49.9149 2.12136 0.0112172 14 0.351546 1
0.355606 0.617249 0.641014 0.0817144 0.768587 0.0553125 49.245 2.08098 0.0120907 13.8757 0.343086 1*/
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
