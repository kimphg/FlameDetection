#ifndef VIDEOWORK_H
#define VIDEOWORK_H

#include <QObject>
#include <QtCore>
#include <QMutex>
#include <QMessageBox>

#include "common.h"
#include "utils.h"
#include "FlameDetector.h"
#include "Config.h"
#include "videowork.h"


class VideoWork : public QObject
{
    Q_OBJECT

public:
    Mat     m_Frame;
    bool    m_IsFinished;
public:
    explicit VideoWork(QObject *parent = 0);
    void requestWork();
    void abort();

private:
    bool    m_abort;
    bool    m_working;
    QMutex  m_mutex;

    FlameDetector mDetector;

signals:
    void workRequested();
    void finished();

public slots:
    void doWork();

signals:

public slots:
};

#endif // VIDEOWORK_H
