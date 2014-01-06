#ifndef TCPBACKEND_H
#define TCPBACKEND_H

#include "basebackend.h"
#include "../../Debug-Server/Debug_Server/debugserver_protocol.h"

#include <QString>
#include <QStringList>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>
#include <QDebug>

class TcpBackend : public BaseBackend {
    Q_OBJECT

protected:
    QHostAddress * hostAddress;
    qint16 port;
    QString devicePath;
    bool writeAccessGranted;

protected slots:
    void socketConnected(void);
    void socketDisconnected(void);


public:
    TcpBackend(QObject * parent = 0);

    static const QString connectionPrefix;
    virtual bool isReadOnly(void) const;
    void setWriteAccess(bool granted);
    virtual QString getConnectionInfo();

public slots:
    virtual bool openConnection(QString connectionString);
    void onTcpError(QAbstractSocket::SocketError err);

};

#endif // TCPBACKEND_H
