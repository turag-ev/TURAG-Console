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
    virtual bool isBuffered(void) const { return false; }
    void setWriteAccess(bool granted);
    virtual QString getConnectionInfo();

public slots:
    virtual bool openConnection(QString connectionString);
    virtual void checkData(void);

protected:
    QHostAddress * hostAddress;
    qint16 port;
    QString devicePath;
    bool writeAccessGranted;

protected slots:
    void socketConnected(void);
    void socketDisconnected(void);
    void onTcpError(QAbstractSocket::SocketError err);
    void emitData(void);


};

#endif // TCPBACKEND_H
