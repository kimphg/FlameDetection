//
//  FeatureAnalyzer.h
//  FlameDetection
//
//  Created by liberize on 14-4-14.
//  Copyright (c) 2014年 liberize. All rights reserved.
//

#ifndef __FlameDetection__FeatureAnalyzer__
#define __FlameDetection__FeatureAnalyzer__

#include "common.h"

struct Target;
class Region;


class Feature {
private:
    Mat mROI;
    Mat mTargetFrame;
    Mat mMask;
    double mArea;

    static const int MAX_AREA_VEC_SIZE = 64;
    
    // if we use list here, we need to convert it to vector each time. so use vector instead.
    vector<double> mAreaVec;
    vector<double> aspectRatioVec;
    vector<double> circularityVec;
    vector<double> squarenessVec;
    void calcColorFeature();
    void calcGeometryFeature(const Region& region);
    void calcTexture(int levels = 16, int dx = 3, int dy = 3);
    void calcFrequency();
    void calcDynamicFeatures();
    
public:
    static const int LEN = 13;
    bool dataReady;
    // color features
    //double red[4];
    //double gray[4];
    //double saturation[4];
    
    // geometric features

    double roughness;
    double diffInOut;
    // structural features
    double texture[4];
    
    // dynamic features
    double aspectRatioMean, aspectRatioVar;
    double circularityMean, circularityVar;
    double squarenessMean,  squarenessVar;
    double frequency;
    double areaVar;
    void calc(const Region& region, const Mat& frame);
    static void merge(const vector<const Feature*>& src, Feature& feature);
    void printValue()const;
    operator Mat() const;

public:
    Feature();

#ifdef PHUONGS_ALGORITHM
    void printAreaVec() const;
#endif
};

ifstream& operator>>(ifstream& ifs, Feature& feature);
ofstream& operator<<(ofstream& ofs, const Feature& feature);


class FeatureAnalyzer {
private:
    Mat mFrame;
    
    void featureUpdate(Target& target);
    void featureMerge(Target& target, const map<int, Target>& targets, const vector<int>& keys);
    void targetUpdate(map<int, Target>& targets);

public:
    void analyze(const Mat& frame, map<int, Target>& targets);
};

#endif /* defined(__FlameDetection__FeatureAnalyzer__) */
