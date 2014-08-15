#include "tcpbackend.h"

#include <QDebug>
#include <QStringList>

const QString TcpBackend::connectionPrefix = "tcp://";

TcpBackend::TcpBackend (QObject *parent) :
    BaseBackend(TcpBackend::connectionPrefix, parent), connecting(false)
{ }

bool TcpBackend::openConnection(QString connectionString) {
    if (!canHandleUrl(connectionString)) {
        return false;
    }

    // prevent another connection attempt while already connecting
    // that's required here because ot the asynchronous character of
    // underlying tcp socket
    if (connecting) return false;

    connecting = true;

    // close connection in case we had one open
    if (isOpen()) closeConnection();

    connectionString_ = connectionString;

    //ich überprüfe den connectionString nicht weiter, da er maschinell erzeugt wird
    //erstmal connectionPrefix wegwerfen:
    connectionString.remove(0, connectionPrefix.length());

    //was ich jetzt noch hab: host:port/path:description
    int index = connectionString.indexOf("/");
    int index2 = connectionString.indexOf(":");
    int index3 = connectionString.indexOf(":", index2 + 1);
    devicePath = connectionString.mid(index + 1, index3 - index - 1);
    QString host = connectionString.left(index);

    QStringList addressAndPort = host.split(":");

    hostAddress = new QHostAddress(addressAndPort.at(0));
    port = addressAndPort.at(1).toInt();

    //und jetzt den Socket erzeugen
    std::unique_ptr<QTcpSocket>  socket(new QTcpSocket);
    connect(socket.get(), SIGNAL(readyRead()), this, SLOT(emitDataReady()));
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
    connecting = false;
    emitConnected();

    QString connectionString = connectionString_;

    connectionString.remove(0, connectionPrefix.length());

    int index = connectionString.indexOf("/");
    int index2 = connectionString.indexOf(":");
    int index3 = connectionString.indexOf(":", index2 + 1);
    QString path = connectionString.mid(index + 1, index3 - index - 1);

    emit requestData(path);

}

void TcpBackend::socketDisconnected(void) {
    connecting = false;
}

void TcpBackend::onTcpError(QAbstractSocket::SocketError error) {
    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
        if (stream_->isOpen() && !connecting) connectionWasLost();
        connecting = false;
        stream_->close();
        stream_->disconnect(this);

        logFilteredErrorMsg("Connection refused");
        break;
    case QAbstractSocket::HostNotFoundError:
        if (stream_->isOpen() && !connecting) connectionWasLost();
        connecting = false;
        stream_->close();
        stream_->disconnect(this);

        logFilteredErrorMsg("Host not found");
        break;
    case QAbstractSocket::RemoteHostClosedError:
        if (stream_->isOpen() && !connecting) connectionWasLost();
        connecting = false;
        stream_->close();
        stream_->disconnect(this);

        logFilteredErrorMsg("Remote host closed");
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
        return false;
    }
}



QString TcpBackend::getConnectionInfo() {
    QString connectionString = connectionString_;

    connectionString.remove(0, connectionPrefix.length());

    int index = connectionString.indexOf(":");
    int index2 = connectionString.indexOf(":", index + 1);
    QString path = connectionString.right(connectionString.size() - index2 - 1);

    return QString("Debug-Server: %1").arg(path);
}

void TcpBackend::closeConnection(void) {
    connecting = false;
    BaseBackend::closeConnection();
}
