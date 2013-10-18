#include "filebackend.h"
#include <QFile>
#include <QApplication>
#include <QFileInfo>
#include <QFileSystemWatcher>

const QString FileBackend::connectionPrefix = "file://";


FileBackend::FileBackend(QObject *parent) :
    BaseBackend(parent)
{
    watcher = new QFileSystemWatcher(this);
}


bool FileBackend::openConnection(QString connectionString) {
    if (!connectionString.startsWith(FileBackend::connectionPrefix)) {
        return false;
    }

    // close connection in case we had one open
    closeConnection();

    // extract filename
    QString newConnectionString = connectionString.right(connectionString.length() - FileBackend::connectionPrefix.length());

    // open new file
    stream_.reset(new QFile(newConnectionString));

    bool success = stream_->open(QIODevice::ReadOnly);
    if (!success) {
      emit errorOccured(QString("Fehler beim Öffnen von Datei: %1").arg(stream_->errorString()));
      return false;
    }

    checkData();

    watcher->addPath(newConnectionString);
    connect(watcher,SIGNAL(fileChanged(QString)),this,SLOT(onFileChanged()));

    connectionString_ = connectionString;

    emit connected(!stream_->isWritable(), stream_->isSequential());

    return true;
}


void FileBackend::closeConnection(void) {
    BaseBackend::closeConnection();
    watcher->disconnect(this);
    if (!connectionString_.isEmpty()) {
        watcher->removePath(connectionString_.right(connectionString_.length() - FileBackend::connectionPrefix.length()));
    }
}


void FileBackend::checkData(void) {
    if (stream_.get() && stream_->isReadable()) {
        emit infoMessage("Datei wird gelesen");
        QApplication::processEvents();

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
        QString newConnectionString = connectionString_.right(connectionString_.length() - FileBackend::connectionPrefix.length());
        return QFileInfo(newConnectionString).fileName();
    }
}
