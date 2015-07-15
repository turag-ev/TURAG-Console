#include "tcpbackend.h"

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

BaseBackend::ConnectionStatus TcpBackend::doOpenConnection(QUrl url) {
	QTcpSocket* socket = new QTcpSocket;
	connect(socket, SIGNAL(readyRead()), this, SLOT(emitDataReady()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onTcpError(QAbstractSocket::SocketError)));
	connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

	socket->connectToHost(url.host(), url.port());

    /*ob die connection erfolgreich war, lässt sich an dieser Stelle leider nicht ermitteln
     *VOID QTcpSocket::connectToHost(), sollte dennoch ein Fehler auftreten, so wird er schlicht
     *ausgegeben */

	stream_.reset(socket);

	return ConnectionStatus::ongoing;
}


void TcpBackend::socketConnected(void) {
	connectingSuccessful();

	emit requestData(connectionUrl_.path());
}

void TcpBackend::socketDisconnected(void) {
	// when is this called????
	//connecting = false;
}

void TcpBackend::onTcpError(QAbstractSocket::SocketError error) {
    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
		if (stream_->isOpen() && !isConnectionInProgress()) {
			connectionWasLost();
		} else {
			connectingFailed();
		}

        logFilteredErrorMsg("Connection refused");
        break;
    case QAbstractSocket::HostNotFoundError:
		if (stream_->isOpen() && !isConnectionInProgress()) {
			connectionWasLost();
		} else {
			connectingFailed();
		}

        logFilteredErrorMsg("Host not found");
        break;
    case QAbstractSocket::RemoteHostClosedError:
		if (stream_->isOpen() && !isConnectionInProgress()) {
			connectionWasLost();
		} else {
			connectingFailed();
		}

        logFilteredErrorMsg("Remote host closed");
        break;
    default:
        qDebug() << error << stream_->errorString();
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

