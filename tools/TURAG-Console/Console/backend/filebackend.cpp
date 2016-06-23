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

FileBackend::~FileBackend() {
	doCleanUpConnection();
}


bool FileBackend::doConnectionPreconditionChecking(const QUrl& url) {
	if (url.path().isEmpty()) {
		logFilteredErrorMsg("no path specified");
		return false;
	}
	if (!QFile::exists(url.path())) {
		logFilteredErrorMsg("file does not exist");
		return false;
	}
	return true;
}

BaseBackend::ConnectionStatus FileBackend::doOpenConnection(const QUrl &url) {
	stream_.reset(new QFile(url.path()));

    bool success = stream_->open(QIODevice::ReadOnly);
    if (!success) {
		logFilteredErrorMsg(QString("Fehler beim Öffnen von Datei: %1").arg(stream_->errorString()));
		return ConnectionStatus::failed;
    }

	watcher->addPath(url.path());
	connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(onFileChanged()));

	// call connectingSuccessful() because it clears the buffer
	connectingSuccessful();

	buffer.append(stream_->readAll());
    logFilteredInfoMessage("Datei gelesen");

	return ConnectionStatus::successful;
}

void FileBackend::doCleanUpConnection(void) {
	watcher->disconnect(this);
	if (!connectionUrl_.isEmpty()) {
		watcher->removePath(connectionUrl_.path());
	}
}


void FileBackend::onFileChanged() {
	if (!stream_.data() || !static_cast<QFile*>(stream_.data())->exists()) {
        logFilteredErrorMsg("Datei existiert nicht mehr");
        connectionWasLost();
    } else {
        BaseBackend::openConnection();
    }
}


QString FileBackend::getConnectionInfo() const {
    if (connectionUrl_.isEmpty()) {
        return "";
    } else {
		QString file = connectionUrl_.path();

		// strip filename from path
		return QFileInfo(file).fileName();
    }
}
