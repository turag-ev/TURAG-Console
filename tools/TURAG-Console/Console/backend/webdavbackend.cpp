#include "webdavbackend.h"
#include <QUrl>
#include <qt/qwebdav/qwebdavlib/qwebdav.h>
#include <libs/log.h>


WebDAVBackend::WebDAVBackend(bool ignoreSslErrors_, QObject *parent) :
	BaseBackend({"http", "https"}, parent), ignoreSslErrors(ignoreSslErrors_)
{
	connect(&webdav, SIGNAL(errorChanged(QString)), this, SLOT(errorOccured(QString)));
}

WebDAVBackend::~WebDAVBackend() {
	doCleanUpConnection();
}



QString WebDAVBackend::getConnectionInfo() const {
	if (connectionUrl_.isEmpty()) {
		return "";
	} else {
		QUrl url(connectionUrl_);
		return url.fileName();
	}
}

bool WebDAVBackend::isOpen(void) const {
	return !stream_.isNull() && !isConnectionInProgress();
}

bool WebDAVBackend::doConnectionPreconditionChecking(const QUrl& ) {
	return true;
}

BaseBackend::ConnectionStatus WebDAVBackend::doOpenConnection(QUrl url) {
	QWebdav::QWebdavConnectionType connectionType;
	if (url.scheme() == "http") {
		connectionType = QWebdav::HTTP;
	} else if (url.scheme() == "https") {
		connectionType = QWebdav::HTTPS;
	} else {
		return ConnectionStatus::failed;
	}

	webdav.setConnectionSettings(connectionType, url.host(), url.path(), url.userName(), url.password(), url.port());
	webdav.setAlwaysIgnoreSslErrors(ignoreSslErrors);

	QNetworkReply* reply = webdav.get("");

	connect(reply, SIGNAL(readyRead()), this, SLOT(onDataReady()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyError(QNetworkReply::NetworkError)));
	connect(reply, SIGNAL(finished()), this, SLOT(finished()));

	stream_.reset(reply);

	return ConnectionStatus::ongoing;
}

void WebDAVBackend::doCleanUpConnection(void) {
	if (stream_.data() != nullptr) {
		QNetworkReply* reply = static_cast<QNetworkReply*>(stream_.data());

		if (!reply->isFinished()) {
			reply->abort();
		}
	}
}

void WebDAVBackend::onDataReady(void) {
	if (isConnectionInProgress()) {
		connectingSuccessful();
	}

	emitDataReady();
}

void WebDAVBackend::errorOccured(QString msg) {
	Log::critical(msg);

	closeConnection();
}

void WebDAVBackend::replyError(QNetworkReply::NetworkError) {
	QNetworkReply* reply = static_cast<QNetworkReply*>(stream_.data());

	Log::critical(reply->error() + ": " + reply->errorString());
	qDebug() << reply->errorString();

	closeConnection();
}

// workaround for zero-byte files
void WebDAVBackend::finished(void) {
	if (isConnectionInProgress()) {
		connectingSuccessful();
	}
}

