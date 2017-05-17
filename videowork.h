#ifndef VIDEOWORK_H
#define VIDEOWORK_H

#include <QObject>
#include <QtCore>
#include <QMutex>
#include <QMessageBox>
#include "QSound"

#include "common.h"
#include "utils.h"
#include "Config.h"
//#include "videowork.h"
#include "FlameDetector.h"


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

private:
    bool    m_abort;
    bool    m_working;
    QMutex  m_mutex;
    FlameDetector mDetector;
    Mat     m_Frame;

signals:
    void workRequested();
    void finished();



public slots:
    void doWork();
    void doWork2();
    void doWork3();

signals:

public slots:
};

#endif // VIDEOWORK_H
