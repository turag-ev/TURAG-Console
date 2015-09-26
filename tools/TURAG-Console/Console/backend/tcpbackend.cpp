#include "tcpbackend.h"
#include <libs/tcpsocketext.h>

#include <QDebug>
#include <QStringList>

const QString TcpBackend::protocolScheme = "tcp";


TcpBackend::TcpBackend (QObject *parent) :
	BaseBackend({TcpBackend::protocolScheme}, parent)
{ }

TcpBackend::~TcpBackend() {
}


bool TcpBackend::doConnectionPreconditionChecking(const QUrl& url) {
	if (url.port() == -1) {
		logFilteredErrorMsg("no port specified");
		return false;
	}
	if (url.host().isEmpty()) {
		logFilteredErrorMsg("no host specified");
		return false;
	}
	return true;
}

BaseBackend::ConnectionStatus TcpBackend::doOpenConnection(const QUrl& url) {
	TcpSocketExt* socket = new TcpSocketExt(true);
	connect(socket, SIGNAL(readyRead()), this, SLOT(emitDataReady()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onTcpError(QAbstractSocket::SocketError)));
	connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

    stream_.reset(socket);
    socket->connectToHost(url.host(), url.port());

	return ConnectionStatus::ongoing;
}


void TcpBackend::socketConnected(void) {
	connectingSuccessful();
}

void TcpBackend::socketDisconnected(void) {
	// when is this called????
	//connecting = false;
}

void TcpBackend::onTcpError(QAbstractSocket::SocketError error) {
    qDebug() << error << stream_->errorString();

    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
	case QAbstractSocket::HostNotFoundError:
	case QAbstractSocket::RemoteHostClosedError:
	case QAbstractSocket::NetworkError:
		logFilteredErrorMsg(stream_->errorString());
		if (stream_->isOpen() && !isConnectionInProgress()) {
			connectionWasLost();
		} else {
			connectingFailed();
		}

        break;

    default:
        break;
    }
}


bool TcpBackend::isReadOnly(void) const {
	if (stream_.data() == nullptr) {
        return true;
    } else {
        return false;
    }
}


QString TcpBackend::getConnectionInfo() const {
	return QString("Debug-Server: %1").arg(connectionUrl_.fragment());
}

QString TcpBackend::getDevicePath(void) const {
	return connectionUrl_.path();
}

