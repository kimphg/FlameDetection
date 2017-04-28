#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QtNetwork>
#include <QObject>
#include <QString>
#include <QTcpSocket>


class TCPClient : public QThread
{
    Q_OBJECT    
public:
    bool bConnected;
public:
    explicit TCPClient(QObject *parent = 0);
    ~TCPClient();
    void start(QString address, quint16 port);
    void startTransfer();
signals:

public slots:    
    void connectedSuccess();
private:
    QTcpSocket client;
};

#endif // TCPCLIENT_H
