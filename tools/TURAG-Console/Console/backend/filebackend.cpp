#include "filebackend.h"
#include <QFile>
#include <QApplication>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QUrl>

const QString FileBackend::protocolScheme = "file";


FileBackend::FileBackend(QObject *parent) :
	BaseBackend({FileBackend::protocolScheme}, parent)
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
	QUrl url(connectionString);
	if (!url.isValid()) {
		return false;
	}

	QString file = url.path();

    // open new file
	stream_.reset(new QFile(file));

    bool success = stream_->open(QIODevice::ReadOnly);
    if (!success) {
      logFilteredErrorMsg(QString("Fehler beim Öffnen von Datei: %1").arg(stream_->errorString()));
      return false;
    }

	watcher->addPath(file);
    connect(watcher,SIGNAL(fileChanged(QString)),this,SLOT(onFileChanged()));

    connectionString_ = connectionString;

    emitConnected();

	buffer.append(stream_->readAll());
    logFilteredInfoMessage("Datei gelesen");

    return true;
}


void FileBackend::closeConnection(void) {
    BaseBackend::closeConnection();
    watcher->disconnect(this);
    if (!connectionString_.isEmpty()) {
		QUrl url(connectionString_);
		watcher->removePath(url.path());
    }
}


void FileBackend::onFileChanged() {
	if (!static_cast<QFile*>(stream_.data())->exists()) {
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
		QUrl url(connectionString_);
		QString file = url.path();
		return QFileInfo(file).fileName();
    }
}
