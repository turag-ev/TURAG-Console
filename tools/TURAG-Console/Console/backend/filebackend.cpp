#include "filebackend.h"
#include <QFile>
#include <QApplication>
#include <QFileInfo>
#include <QFileSystemWatcher>

const QString FileBackend::connectionPrefix = "file://";
const bool FileBackend::networked = false;


FileBackend::FileBackend(QObject *parent) :
    BaseBackend(FileBackend::connectionPrefix, networked, parent)
{
    watcher = new QFileSystemWatcher(this);
}


bool FileBackend::openConnection(QString connectionString) {
    if (!canHandleUrl(connectionString)) {
        return false;
    }

    // close connection in case we had one open
    closeConnection();

    // extract filename
    QString newConnectionString = connectionString.right(connectionString.length() - connectionPrefix.length());

    // open new file
    stream_.reset(new QFile(newConnectionString));

    bool success = stream_->open(QIODevice::ReadOnly);
    if (!success) {
      emit errorOccured(QString("Fehler beim Öffnen von Datei: %1").arg(stream_->errorString()));
      return false;
    }

    watcher->addPath(newConnectionString);
    connect(watcher,SIGNAL(fileChanged(QString)),this,SLOT(onFileChanged()));

    connectionString_ = connectionString;

    emitConnected();

    return true;
}


void FileBackend::closeConnection(void) {
    BaseBackend::closeConnection();
    watcher->disconnect(this);
    if (!connectionString_.isEmpty()) {
        watcher->removePath(connectionString_.right(connectionString_.length() - connectionPrefix_.length()));
    }
}


void FileBackend::checkData(void) {
    if (stream_.get() && stream_->isReadable()) {

        // process events caused the application to crash.
//        emit infoMessage("Datei wird gelesen");
//        QApplication::processEvents();

        stream_->seek(0);
        emit dataReady(stream_->readAll());

        emit infoMessage("Datei gelesen");
    }
}


void FileBackend::onFileChanged() {
    if (!static_cast<QFile*>(stream_.get())->exists()) {
        emit errorOccured("Datei existiert nicht mehr");
        closeConnection();
    } else {
        checkData();
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
