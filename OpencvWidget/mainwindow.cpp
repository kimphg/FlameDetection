#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#define LINE_PROLONG_RATIO 1.7
#define MAX_LINES 30
#define ANGLE_MIN 1.04
#define ANGLE_MAX 2.26
#define ANGLE_2_MIN 0.154//(pi/8)^2
#define MIN_ANGLE_LINES 0.1
#define MIN_DIAG_LEN 300
using namespace std;
using namespace cv;
typedef struct
{
    Vec2i   v1;
    Vec2i   v2;
    Vec2i   center;
    double linesLen;
    double angle;
}RectCorner;
double innerAngleOfLine(Vec2i v1,Vec2i v2,Vec2i v3, Vec2i v4)
{
    double dx21 = v2[0]-v1[0];
    double dx43 = v4[0]-v3[0];
    double dy21 = v2[1]-v1[1];
    double dy43 = v4[1]-v3[1];
    double m12 = sqrt( dx21*dx21 + dy21*dy21 );
    double m34 = sqrt( dx43*dx43 + dy43*dy43 );
    double a=(dx21*dx43 + dy21*dy43) / (m12 * m34);
    if (a >= 1.0)
        return 0.0;
    else if (a <= -1.0)
        return 0.0;
    else
    {
        double res = acos(a); // 0..PI
        if(res>3.14159265358979/2.0)res = 3.14159265358979-res;
        return res;
    }
}

double angleBetween(const Vec2i v1,const Vec2i center, const Vec2i v2)
{
    int v10 = v1[0]-center[0];
    int v11 = v1[1]-center[1];
    int v20 = v2[0]-center[0];
    int v21 = v2[1]-center[1];
    double len1 = sqrt(v10 * v10 + v11 * v11);
    double len2 = sqrt(v20 * v20 + v21 * v21);
    double a = (v10 * v20 + v11 * v21 )/ (len1 * len2);
    if (a >= 1.0)
        return 0.0;
    else if (a <= -1.0)
        return 3.1415926535989;
    else
        return acos(a); // 0..PI
}
bool compareContourAreas ( std::vector<cv::Point> contour1, std::vector<cv::Point> contour2 ) {
    double i = fabs( contourArea(cv::Mat(contour1)) );
    double j = fabs( contourArea(cv::Mat(contour2)) );
    return ( i > j );
}
double distance1L(Vec4i line)
{
    int dx=abs(line[0]-line[2]);
    int dy=abs(line[1]-line[3]);
    return sqrt(dx*dx+dy*dy);
}
double distance2P(Vec2i p1,Vec2i p2)
{
    int dx=abs(p1[0]-p2[0]);
    int dy=abs(p1[1]-p2[1]);
    return sqrt(dx*dx+dy*dy);
}
double distance4P(int x1, int y1, int x2, int y2)
{
    int dx=abs(x1-x2);
    int dy=abs(y1-y2);
    return sqrt(dx*dx+dy*dy);
}
Vec2i getIntersect(Vec2i v1,Vec2i v2,Vec2i v3,Vec2i v4)
{
    int x1 = v1[0];
    int y1 = v1[1];
    int x2 = v2[0];
    int y2 = v2[1];
    int x3 = v3[0];
    int y3 = v3[1];
    int x4 = v4[0];
    int y4 = v4[1];
    int d = (x1-x2)*(y3-y4) - (y1-y2)*(x3-x4);
    Vec2i res(0,0);
    if (d == 0)
        return res;
    res[0] = ((x3-x4)*(x1*y2-y1*x2)-(x1-x2)*(x3*y4-y3*x4))/d;
    res[1] = ((y3-y4)*(x1*y2-y1*x2)-(y1-y2)*(x3*y4-y3*x4))/d;
    return res;
}

bool cornerCheck(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4,RectCorner* corner) {
    int d = (x1-x2)*(y3-y4) - (y1-y2)*(x3-x4);
    if (d == 0) return false;
    int ctx = corner->center[0] = ((x3-x4)*(x1*y2-y1*x2)-(x1-x2)*(x3*y4-y3*x4))/d;
    int cty = corner->center[1] = ((y3-y4)*(x1*y2-y1*x2)-(y1-y2)*(x3*y4-y3*x4))/d;
    double d1=distance4P(ctx,cty,x1,y1);
    double d2=distance4P(ctx,cty,x2,y2);
    double d3=distance4P(ctx,cty,x3,y3);
    double d4=distance4P(ctx,cty,x4,y4);
    if(int(d1*d2*d3*d4)==0)return false;
    if(d1>d2)
    {
        if(d1/d2<LINE_PROLONG_RATIO)return false;
        corner->v1[0] = x1;//-int(corner->center[0]);
        corner->v1[1] = y1;//-int(corner->center[1]);
    }
    else
    {
        if((d2/d1)<LINE_PROLONG_RATIO)return false;
        corner->v1[0] = x2;//-int(corner->center[0]);
        corner->v1[1] = y2;//-int(corner->center[1]);
        d1=d2;
    }
    if(d3>d4)
    {
        if((d3/d4)<LINE_PROLONG_RATIO)return false;
        corner->v2[0] = x3;//-int(corner->center[0]);
        corner->v2[1] = y3;//-int(corner->center[1]);
    }
    else
    {
        if((d4/d3)<LINE_PROLONG_RATIO)return false;
        corner->v2[0] = x4;//-int(corner->center[0]);
        corner->v2[1] = y4;//-int(corner->center[1]);
        d3=d4;
    }
    double angle = angleBetween(corner->v1,corner->center,corner->v2);
    if(angle<ANGLE_MIN||angle>ANGLE_MAX)return false;
    corner->angle=angle;
    corner->linesLen = d1+d3;
    return true;
    //return Point2f(xi,yi);
}
bool isInsideCorner(RectCorner corner, Vec2i point, double amin)
{
    double a1 = angleBetween(corner.v1,corner.center,point);
    double a2 = angleBetween(corner.v2,corner.center,point);
    if(a1<amin||a2<amin)return false;
    if(a1+a2>corner.angle+0.1
            )
    {
        return false;
    }
    return true;
}
bool CheckROI(RectCorner* corner1,RectCorner *corner2,Point2f roi[])
{
    double diag = distance2P(corner2->center,corner1->center);
    if(diag<MIN_DIAG_LEN)return false;

    if(
            isInsideCorner(*corner1,corner2->center,0.4)
            &&isInsideCorner(*corner1,corner2->v1,0)
            &&isInsideCorner(*corner1,corner2->v2,0)
            &&isInsideCorner(*corner2,corner1->center,0.4)
            &&isInsideCorner(*corner2,corner1->v1,0)
            &&isInsideCorner(*corner2,corner1->v2,0)
            )
    {
        double a11 = (innerAngleOfLine(corner1->center,corner1->v1,corner2->center,corner2->v1));
        double a22 = (innerAngleOfLine(corner1->center,corner1->v2,corner2->center,corner2->v2));
        double a12 = (innerAngleOfLine(corner1->center,corner1->v1,corner2->center,corner2->v2));
        double a21 = (innerAngleOfLine(corner1->center,corner1->v2,corner2->center,corner2->v1));
        if(a11<MIN_ANGLE_LINES)
        {
            if(a22<MIN_ANGLE_LINES)
            {
                roi[0] = Point2f(corner1->center);
                roi[1] = Point2f(getIntersect(corner1->center,corner1->v1,corner2->center,corner2->v2));
                roi[2] = Point2f(corner2->center);
                roi[3] = Point2f(getIntersect(corner1->center,corner1->v2,corner2->center,corner2->v1));
                return true;

            }
        }
        else if(a12<MIN_ANGLE_LINES)
        {
            if(a21<MIN_ANGLE_LINES)
            {
                roi[0] = Point2f(corner1->center);
                roi[1] = Point2f(getIntersect(corner1->center,corner1->v1,corner2->center,corner2->v1));
                roi[2] = Point2f(corner2->center);
                roi[3] = Point2f(getIntersect(corner1->center,corner1->v2,corner2->center,corner2->v2));
                return true;
            }
        }
    }
    return false;
}
bool mergeLines(Vec4i &line1,Vec4i &line2)
{

    Vec2i v1 = Vec2i(&line1[0]);
    Vec2i v2 = Vec2i(&line1[2]);
    Vec2i v3 = Vec2i(&line2[0]);
    Vec2i v4 = Vec2i(&line2[2]);
    //check inner angle
    double angle = innerAngleOfLine(v1,v2,v3,v4);
    if(angle>MIN_ANGLE_LINES/2.0)return false;
    //check point distance
    double d1 = distance1L(line1);
    double dmax = distance1L(line2);
    if(dmax<d1)dmax=d1;
    dmax/=5.0;

    double d13 = distance2P(v1,v3);
    double d14 = distance2P(v1,v4);
    double d23 = distance2P(v2,v3);
    double d24 = distance2P(v2,v4);
    if(d13<dmax)
    {
        double angle = innerAngleOfLine(v1,v3,v1,v2);
        double angle2 = innerAngleOfLine(v2,v4,v1,v2);
        angle=(angle+angle2)/2;
        if(angle>MIN_ANGLE_LINES)return false;
        line1[0] = v2[0];
        line1[1] = v2[1];
        line1[2] = v4[0];
        line1[3] = v4[1];
    }
    if(d14<dmax)
    {
        double angle = innerAngleOfLine(v1,v4,v1,v2);
        double angle2 = innerAngleOfLine(v2,v3,v1,v2);
        angle=(angle+angle2)/2;
        if(angle>MIN_ANGLE_LINES)return false;
        line1[0] = v2[0];
        line1[1] = v2[1];
        line1[2] = v3[0];
        line1[3] = v3[1];
    }
    if(d23<dmax)
    {
        double angle = innerAngleOfLine(v2,v3,v1,v2);
        double angle2 = innerAngleOfLine(v1,v4,v1,v2);
        angle=(angle+angle2)/2;
        if(angle>MIN_ANGLE_LINES)return false;
        line1[0] = v1[0];
        line1[1] = v1[1];
        line1[2] = v4[0];
        line1[3] = v4[1];
    }
    if(d24<dmax)
    {
        double angle = innerAngleOfLine(v2,v4,v1,v2);
        double angle2 = innerAngleOfLine(v1,v3,v1,v2);
        angle=(angle+angle2)/2;
        if(angle>MIN_ANGLE_LINES)return false;
        line1[0] = v1[0];
        line1[1] = v1[1];
        line1[2] = v4[0];
        line1[3] = v4[1];
    }
    return true;
}
void reArrange(Point2f roi[])//make roi clock wise and start from smallest
{
    while (true) {
        double d0 = distance4P(roi[0].x,roi[0].y,0,0);
        double d1 = distance4P(roi[1].x,roi[1].y,0,0);
        double d2 = distance4P(roi[2].x,roi[2].y,0,0);
        double d3 = distance4P(roi[3].x,roi[3].y,0,0);
        if(d0>d1||d0>d2||d0>d3)
        {
            Point2f temp = roi[0];
            roi[0]=roi[1];
            roi[1]=roi[2];
            roi[2]=roi[3];
            roi[3]=temp;
            continue;
        }
        break;

    }
    if(roi[1].x<roi[3].x)
    {
        Point2f temp = roi[1];
        roi[1]=roi[3];
        roi[3]=temp;
    }
}
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
        return;
    Mat imgProcess;
    //Mat kernel;
    //int kernel_size = 50;
    //kernel = Mat::zeros(kernel_size,kernel_size,CV_32F);
    /*float *kernel_pointer=(float*)kernel.data;
    for(int i=0;i<kernel_size;i++)
    {
        (kernel_pointer+(kernel_size*kernel_size/2))[i]= 1/50.0f;
    }
    cout<<kernel;
    Point2f src_center(kernel_size/2.0F, kernel_size/2.0F);
    Mat rot_mat = getRotationMatrix2D(src_center, 90, 1.0);
    Mat kernel2;
    warpAffine(kernel, kernel2, rot_mat,kernel.size());*/
    //namedWindow("edges",1);
    Mat acc;
    Mat result;
    for(;;)
    {
        Mat originFrame;
        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;

        cap >> originFrame; // get a new frame from camera
        if(originFrame.empty())continue;
        /*if(acc.empty())acc = Mat::zeros(originFrame.size(), CV_32F);
        else{
            acc = cv::Mat(acc*0.666666667);
            accumulate(originFrame*0.33333333,acc);
            acc.convertTo(originFrame,originFrame.type());

        }*/

        int frameW=originFrame.size().width;
        int frameH=originFrame.size().height;
        cvtColor(originFrame, imgProcess, COLOR_BGR2GRAY);
        GaussianBlur(imgProcess, imgProcess, Size(9,9), 3, 3);
        imshow("imgProcess",imgProcess);

        //equalizeHist(imgProcess,imgProcess);
        Canny(imgProcess, imgProcess, 30, 50, 3);
        //dilate(imgProcess, imgProcess, Mat(), Point(-1,-1));
        /*findContours( imgProcess, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );
        imgProcess = Mat::zeros(frameH,frameW,CV_8U);
        for(  uint i = 0; i<contours.size(); i++ )
        {
            Scalar color = Scalar(255);
            if(contourArea(cv::Mat(contours[i]))<5)continue;
            drawContours( imgProcess, contours, i, color );

        }*/
        imshow("contours",imgProcess);
        vector<Vec4i> lines;
        Mat HoughLinesPFrame = Mat::zeros(frameH,frameW,CV_8U);
        HoughLinesP(imgProcess, lines, 1, CV_PI/180.0,30,10,5);
        //merge lines
        /*for( size_t i = 0; i<lines.size(); i++ )
        {
            // draw line
            Vec4i l = lines[i];
            line( HoughLinesPFrame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(100), 1, CV_AA);
            for( size_t j = i+1;  j<lines.size(); )
            {
                if(mergeLines(lines[i],lines[j]))
                {
                    lines.erase(lines.begin()+int(j));
                    continue;
                }
                j++ ;
            }
        }
        imshow("before merge",HoughLinesPFrame);*/
        // sorting lines by geometrical length
        size_t numofLines = lines.size()<MAX_LINES?lines.size():MAX_LINES ;
        for( size_t i = 0; i < numofLines; i++ )
        {
            for( size_t j = i+1; j < lines.size(); j++ )
            {
                if(distance4P(lines[j][0],lines[j][1],lines[j][2],lines[j][3])
                        >distance4P(lines[i][0],lines[i][1],lines[i][2],lines[i][3]))
                {
                    Vec4i l1 = lines[i];
                    lines[i] =  lines[j];
                    lines[j] =  l1;
                }

            }
            // draw line
            Vec4i l = lines[i];
            line( HoughLinesPFrame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255), 1, CV_AA);
        }

        // find line intersections
        vector<RectCorner> intersections;

        for( size_t i = 0; i<numofLines; i++ )
        {
            for( size_t j = i+1;  j<numofLines; j++ )
            {

                RectCorner intersect ;

                if(cornerCheck(
                            lines[i][0],lines[i][1],
                            lines[i][2],lines[i][3],
                            lines[j][0],lines[j][1],
                            lines[j][2],lines[j][3],
                            &intersect)
                        )
                {
                    if(
                            intersect.center[0]<(-frameW/10)||
                            intersect.center[0]>(frameW+frameW/10)
                            )continue;
                    if(
                            intersect.center[1]<(-frameH/10)||
                            intersect.center[1]>(frameH+frameH/10)
                            )continue;

                    intersections.push_back(intersect);
                    circle(HoughLinesPFrame, intersect.center, 6, Scalar(255));
                    putText(HoughLinesPFrame,to_string(
                                int(intersect.angle/3.1415926535*180.0)),
                            intersect.center,FONT_HERSHEY_SIMPLEX,0.3,Scalar(255));
                }
            }
        }
        imshow("HoughLinesPFrame",HoughLinesPFrame);
        bool rectOfInterestFound = false;
        double max_len_found = 0;
        for(size_t i=0;i<intersections.size();i++)
        {
            for(size_t j=i+1;j<intersections.size();j++)
            {
                RectCorner *corner1 = &intersections[i];
                RectCorner *corner2 = &intersections[j];
                Point2f roi[4];
                if(CheckROI(corner1,corner2,roi))
                {
                    /*line(HoughLinesPFrame, Point(intersections[i].center[0],intersections[i].center[1]),
                            Point(intersections[j].center[0],intersections[j].center[1]),
                         Scalar(255));*/
                    //Mat dst;
                    Point2f orig[]={Point2f(0,0),
                                    Point2f(originFrame.cols,0),
                                    Point2f(originFrame.cols,originFrame.rows),
                                    Point2f(0,originFrame.rows)};
                    reArrange(roi);
                    Mat _transform_matrix = getPerspectiveTransform(roi,orig);
                    warpPerspective(originFrame, result, _transform_matrix, result.size());
                    rectOfInterestFound = true;
                    double cornerLens = corner1->linesLen+corner2->linesLen;
                    double diag = distance2P(corner2->center,corner1->center);
                    //cout<<"\nlen/diag"<<cornerLens/diag;
                    if(cornerLens<diag)continue;
                    //
                    if(max_len_found>cornerLens)continue;
                    max_len_found=cornerLens;
                    //if(cornerLens/diag>5.0)continue;
                    //max_diag_found = diag;
                    line(originFrame,roi[0],roi[1],Scalar(255));
                    line(originFrame,roi[2],roi[1],Scalar(255));
                    line(originFrame,roi[2],roi[3],Scalar(255));
                    line(originFrame,roi[3],roi[0],Scalar(255));
                    /*Mat b_hist;
                    // Establish the number of bins
                    int histSize = 256;
                    int chanels =1;
                      // Set the ranges ( for B,G,R) )
                    float range[] = { 0, 256 } ;
                    const float* histRange = { range };
                    calcHist(&result, 1,&chanels, Mat(), b_hist, 1, &histSize, &histRange, true, false );
                    double minVal;
                    double maxVal;
                    Point minLoc;
                    Point maxLoc;

                    minMaxLoc( b_hist, &minVal, &maxVal, &minLoc, &maxLoc );
                    cout<<"\nMax histogram:"<<maxVal;
                    // Draw the histograms for B, G and R
                      int hist_w = 512; int hist_h = 400;
                      int bin_w = cvRound( (double) hist_w/histSize );

                      Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

                      /// Normalize the result to [ 0, histImage.rows ]
                      normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

                      for( int i = 1; i < histSize; i++ )
                      {
                          line( histImage, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
                                           Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
                                           Scalar( 255, 0, 0), 2, 8, 0  );

                      }
                      imshow("calcHist Demo", histImage );

                    */
                    imshow("originFrame",originFrame);
                    imshow("HoughLinesPFrame",HoughLinesPFrame);
                    imshow("result",result);


                }
                //find 4 points and its area
            }
        }
        //cout<<"\nintersections:"<<intersections.size();
        //        acc = cv::Mat(acc*2.0/3.0);
        //        accumulate(HoughLinesPFrame*1.0/3.0,acc);
        //        acc.convertTo(HoughLinesPFrame,CV_8U);
        //        threshold(HoughLinesPFrame,HoughLinesPFrame,100,255,CV_8U);

        //        findContours( HoughLinesPFrame, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );
        //cap.set()
        /*filter2D(imgProcess, imgProcess, -1 , kernel );
        imshow("imgCannyKernel",imgProcess);*/
        //acc.convertTo(imgProcess,CV_8U);
        //imshow("HoughLinesPFrame",HoughLinesPFrame);
        /*findContours( HoughLinesPFrame, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );

        std::sort(contours.begin(), contours.end(), compareContourAreas);
        Mat contoursDraw=cv::Mat::zeros(HoughLinesPFrame.size(),CV_8U);
        for(  int i = 0; i< contours.size(); i++ )
             {
               Scalar color = Scalar(255);
               drawContours( contoursDraw, contours, i, color );
               if(i>5)break;
             }
//        Mat kernel;
        imshow("contoursDraw",contoursDraw);*/

        if(waitKey(30) >= 0||rectOfInterestFound)
        {
            if(intersections.size()>4)
                imwrite( "frame.jpg", HoughLinesPFrame );
            break;
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
