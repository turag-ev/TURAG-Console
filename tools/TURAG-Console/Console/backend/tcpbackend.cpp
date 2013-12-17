#include "tcpbackend.h"

const QString TcpBackend::connectionPrefix = "tcp://";

TcpBackend::TcpBackend (QObject *parent) :
    BaseBackend(TcpBackend::connectionPrefix, parent) {

}

bool TcpBackend::openConnection(QString connectionString) {
    if (canHandleUrl(connectionString)) {
        return false;
    }

    // close connection in case we had one open
    closeConnection();

    //ich überprüfe den connectionString nicht weiter, da er maschinell erzeugt wird
    //erstmal connectionPrefix wegwerfen:
    connectionString.remove(0, connectionPrefix.length());

    //was ich jetzt noch hab: host:port/path
    int index = connectionString.indexOf("/");
    QString host;
    QString path;

    int i;
    for (i = 0; i < index; i++) {
        host.append(connectionString.at(i));
    }

    for (i = index + 1; i < connectionString.length(); i++) {
        path.append(connectionString.at(i));
    }

    QStringList addressAndPort = host.split(":");
    if (addressAndPort.size() >= 2) {
        qDebug() << "Host enthält mehr als ein ':' !!" << endl;
        return false;
    }

    hostAddress = new QHostAddress(addressAndPort.at(0));
    port = addressAndPort.at(1).toInt();

    //und jetzt den Socket erzeugen
    client = new QTcpSocket(this);
    connect(client, SIGNAL(readyRead()), this, SLOT(receive()));
    connect(client, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onTcpError(QAbstractSocket::SocketError)));

    client->connectToHost( * hostAddress, port);

    /*ob die connection erfolgreich war, lässt sich an dieser Stelle leider nicht ermitteln
     *VOID QTcpSocket::connectToHost(), sollte dennoch ein Fehler auftreten, so wird er schlicht
     *ausgegeben */

    emit connected(false, client->isSequential());

    return true;
}

void TcpBackend::receive() {
    QByteArray data = client->readAll();
    emit dataReady(data);
}

void TcpBackend::onTcpError(QAbstractSocket::SocketError err) {
    qDebug() << "There is a problem with the data channel tcp connection " << endl <<
                err << endl;
}

void TcpBackend::writeData(QByteArray data) {
    //dataServer liest den socket NICHT zeilenweise ein sondern readAll() und basta
    client->write(data);
}





