#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "documentdetector.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
        return;

    Mat acc;

    for(;;)
    {
        Mat originFrame;
        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
        cap >> originFrame; // get a new frame from camera
        if(originFrame.empty())continue;
        waitKey(30);
        if(detectDocument(originFrame))
        {


        }
        imshow("originFrame",originFrame);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
