#include "filebackend.h"
#include <QFile>
#include <QApplication>
#include <QFileInfo>
#include <QFileSystemWatcher>

const QString FileBackend::connectionPrefix = "file://";


FileBackend::FileBackend(QObject *parent) :
    BaseBackend(FileBackend::connectionPrefix, parent)
{
    watcher = new QFileSystemWatcher(this);
}


bool FileBackend::openConnection(QString connectionString) {
    if (!canHandleUrl(connectionString)) {
        return false;
    }

    // close connection in case we had one open
    if (isOpen()) closeConnection();

    // extract filename
    QString newConnectionString = connectionString.right(connectionString.length() - connectionPrefix.length());

    // open new file
    stream_.reset(new QFile(newConnectionString));

    bool success = stream_->open(QIODevice::ReadOnly);
    if (!success) {
      logFilteredErrorMsg(QString("Fehler beim Öffnen von Datei: %1").arg(stream_->errorString()));
      return false;
    }

    watcher->addPath(newConnectionString);
    connect(watcher,SIGNAL(fileChanged(QString)),this,SLOT(onFileChanged()));

    connectionString_ = connectionString;

    emitConnected();

    buffer->append(stream_->readAll());
    logFilteredInfoMessage("Datei gelesen");

    return true;
}


void FileBackend::closeConnection(void) {
    BaseBackend::closeConnection();
    watcher->disconnect(this);
    if (!connectionString_.isEmpty()) {
        watcher->removePath(connectionString_.right(connectionString_.length() - connectionPrefix_.length()));
    }
}


void FileBackend::onFileChanged() {
    if (!static_cast<QFile*>(stream_.get())->exists()) {
        logFilteredErrorMsg("Datei existiert nicht mehr");
        connectionWasLost();
    } else {
        BaseBackend::openConnection();
    }
}


QString FileBackend::getConnectionInfo() {
    if (connectionString_.isEmpty()) {
        return "";
    } else {
        QString newConnectionString = connectionString_.right(connectionString_.length() - connectionPrefix_.length());
        return QFileInfo(newConnectionString).fileName();
    }
}
