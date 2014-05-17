#ifndef TCPBACKEND_H
#define TCPBACKEND_H

#include "basebackend.h"
#include <Debug_Server/debugserver_protocol.h>

#include <QString>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>

class TcpBackend : public BaseBackend {
    Q_OBJECT

public:
    TcpBackend(QObject * parent = 0);

    static const QString connectionPrefix;
    virtual bool isReadOnly(void) const;
    virtual bool isBuffered(void) const { return true; }
    virtual QString getConnectionInfo();
    QString getDevicePath(void) { return devicePath; }

signals:
    void requestData(QString path);

public slots:
    virtual bool openConnection(QString connectionString);

private slots:
    void socketConnected(void);
    void socketDisconnected(void);
    void onTcpError(QAbstractSocket::SocketError err);

private:
    QHostAddress * hostAddress;
    qint16 port;
    QString devicePath;

    bool connecting;
};

#endif // TCPBACKEND_H
