#include "tcpbackend.h"

const QString TcpBackend::connectionPrefix = "tcp://";

TcpBackend::TcpBackend (QObject *parent) :
    BaseBackend(TcpBackend::connectionPrefix, parent),
    writeAccessGranted(false)
{ }

bool TcpBackend::openConnection(QString connectionString) {
    if (!canHandleUrl(connectionString)) {
        return false;
    }

    // close connection in case we had one open
    closeConnection();

    connectionString_ = connectionString;

    //ich überprüfe den connectionString nicht weiter, da er maschinell erzeugt wird
    //erstmal connectionPrefix wegwerfen:
    connectionString.remove(0, connectionPrefix.length());

    //was ich jetzt noch hab: host:port/path
    int index = connectionString.indexOf("/");
    QString host = connectionString.left(index);

    QStringList addressAndPort = host.split(":");

    hostAddress = new QHostAddress(addressAndPort.at(0));
    port = addressAndPort.at(1).toInt();

    //und jetzt den Socket erzeugen
    std::unique_ptr<QTcpSocket>  socket(new QTcpSocket);
    connect(socket.get(), SIGNAL(readyRead()), this, SLOT(checkData()));
    connect(socket.get(), SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onTcpError(QAbstractSocket::SocketError)));
    connect(socket.get(), SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(socket.get(), SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

    socket->connectToHost( * hostAddress, port);

    /*ob die connection erfolgreich war, lässt sich an dieser Stelle leider nicht ermitteln
     *VOID QTcpSocket::connectToHost(), sollte dennoch ein Fehler auftreten, so wird er schlicht
     *ausgegeben */

    stream_ = std::move(socket);
    return true;
}


void TcpBackend::socketConnected(void) {
    emitConnected();
}

void TcpBackend::socketDisconnected(void) {

}

void TcpBackend::onTcpError(QAbstractSocket::SocketError error) {
    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
        if (stream_->isOpen()) closeConnection();
        emit errorOccured("Connection refused");
        break;
    case QAbstractSocket::HostNotFoundError:
        if (stream_->isOpen()) closeConnection();
        emit errorOccured("Host not found");
        break;
    case QAbstractSocket::RemoteHostClosedError:
        if (stream_->isOpen()) closeConnection();
        emit errorOccured("Remote host closed");
        break;
    default:
        qDebug() << error << stream_->errorString();
        break;
    }

}


bool TcpBackend::isReadOnly(void) const {
    if (stream_.get() == nullptr) {
        return true;
    } else {
        return !writeAccessGranted;
    }
}


void TcpBackend::setWriteAccess(bool granted) {
    writeAccessGranted = granted;

    if (isOpen()) {
        emitConnected();
    }
}


QString TcpBackend::getConnectionInfo() {
    QString connectionString = connectionString_;

    connectionString.remove(0, connectionPrefix.length());

    int index = connectionString.indexOf("/");
    QString path = connectionString.right(connectionString.size() - index - 1);

    return QString("Debug-Server: %1").arg(path);
}

