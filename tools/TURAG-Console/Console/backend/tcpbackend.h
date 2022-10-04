#ifndef TURAG_CONSOLE_CONSOLE_BACKEND_TCPBACKEND_H
#define TURAG_CONSOLE_CONSOLE_BACKEND_TCPBACKEND_H

#include "basebackend.h"

#include <QString>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QAbstractSocket>

class TcpBackend : public BaseBackend {
    Q_OBJECT

public:
    TcpBackend(QObject * parent = 0);
	virtual ~TcpBackend();

    virtual bool isReadOnly(void) const;
	virtual QString getConnectionInfo() const;
	QString getDevicePath(void) const;

	static const QString protocolScheme;

private slots:
    void socketConnected(void);
    void socketDisconnected(void);
    void onTcpError(QAbstractSocket::SocketError err);

private:
	virtual bool doConnectionPreconditionChecking(const QUrl& url);
	virtual BaseBackend::ConnectionStatus doOpenConnection(const QUrl& connectionUrl);

};

#endif // TURAG_CONSOLE_CONSOLE_BACKEND_TCPBACKEND_H
