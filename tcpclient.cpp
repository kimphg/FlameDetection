#include "tcpclient.h"
#include <QHostAddress>
#include <windows.h>


TCPClient::TCPClient(QObject *parent) : QThread(parent)
{
    bConnected = false;
    connect(&client, SIGNAL(connected()), this, SLOT(connectedSuccess()));
}

TCPClient::~TCPClient()
{
  client.close();
}

void TCPClient::start(QString address, quint16 port)
{
    QHostAddress addr(address);
    client.connectToHost(addr, port);
    client.waitForConnected(500);
}

void TCPClient::startTransfer()
{
    if (!bConnected)
        return;

    QByteArray datagram;
    datagram.resize(5);

    datagram[0] = 0xFF;
    datagram[1] = BYTE(0x01 >> 8);
    datagram[2] = BYTE(0x02 );
    datagram[3] = BYTE(0x03 >> 8);
    datagram[4] = BYTE(0x04	   );

    client.write(datagram);
    client.flush();
}

void TCPClient::connectedSuccess()
{
    bConnected = true;
    // Send data testing
    startTransfer();
}


