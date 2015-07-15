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
	virtual ~TcpBackend();

    virtual bool isReadOnly(void) const;
	virtual QString getConnectionInfo() const;
	QString getDevicePath(void) const;

	static const QString protocolScheme;

signals:
    void requestData(QString path);

private slots:
    void socketConnected(void);
    void socketDisconnected(void);
    void onTcpError(QAbstractSocket::SocketError err);

private:
	virtual bool doConnectionPreconditionChecking(const QUrl& url);
	virtual BaseBackend::ConnectionStatus doOpenConnection(QUrl connectionUrl);

};

#endif // TCPBACKEND_H
