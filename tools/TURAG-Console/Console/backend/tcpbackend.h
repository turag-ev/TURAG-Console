#ifndef TCPBACKEND_H
#define TCPBACKEND_H

#include "basebackend.h"
#include "backend/tcpBackend_protocol.h"

#include <QString>
#include <QStringList>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>
#include <QDebug>

class TcpBackend : public BaseBackend {
    Q_OBJECT

protected:

private:
    QHostAddress * hostAddress;
    qint16 port;
    QString devicePath;

    QTcpSocket * client;

protected slots:


public:
    TcpBackend(QObject * parent);

    static const QString connectionPrefix;



public slots:
    virtual bool openConnection(QString connectionString);
    void receive();
    void onTcpError(QAbstractSocket::SocketError err);
    void writeData(QByteArray data);

};

#endif // TCPBACKEND_H
