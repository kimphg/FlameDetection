#ifndef VIDEOWORK_H
#define VIDEOWORK_H

#include <QObject>
#include <QtCore>
#include <QMutex>
#include <QMessageBox>
#include <QNetworkReply>
#include "QSound"

#include "common.h"
#include "utils.h"
#include "Config.h"
//#include "videowork.h"
#include "FlameDetector.h"
#include <QThread>

class Sleeper : public QThread
{
public:
    static void usleep(unsigned long usecs){QThread::usleep(usecs);}
    static void msleep(unsigned long msecs){QThread::msleep(msecs);}
    static void sleep(unsigned long secs){QThread::sleep(secs);}
};

class VideoWork : public QObject
{
    Q_OBJECT

public:

    bool    m_IsFinished;    

public:
    explicit VideoWork(QObject *parent = 0);
    void requestWork();
    void abort();

    bool saveFrame();
    const FlameDetector& getDetector() const { return mDetector; }

    void setTilt(QString ipadr, int angle);
    void setTilt(int angle);
private:

    bool    m_abort;
    bool    m_working;
    QMutex  m_mutex;
    FlameDetector mDetector;
    Mat     m_Frame;
    QNetworkReply *reply;
    QNetworkAccessManager *qnam;

    void StopCamera(QString ipadr);
    void StartCamera(QString ipadr, int rate);
    void resetProgram();
    void closeProgram();

    void moveUp(QString ipadr);
    void moveDown(QString ipadr);
    void moveLeft(QString ipadr);
    void moveRight(QString ipadr);
    void commonWork(std::string url, QString ipadr, std::string winName, int videoPosition, int videoChanel);
signals:
    void workRequested();
    void finished();



public slots:
    void doWork();
    void doWork2();
    void doWork3();

signals:

private slots:
    void onTimer();
};

#endif // VIDEOWORK_H
