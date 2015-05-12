#include "webdavbackend.h"
#include <QUrl>
#include <qwebdav/qwebdavlib/qwebdav.h>
#include <libs/log.h>


WebDAVBackend::WebDAVBackend(bool ignoreSslErrors_, QObject *parent) :
	BaseBackend({"http", "https"}, parent), ignoreSslErrors(ignoreSslErrors_), connecting(false)
{
	connect(&webdav, SIGNAL(errorChanged(QString)), this, SLOT(errorOccured(QString)));
}


QString WebDAVBackend::getConnectionInfo() {
	if (connectionString_.isEmpty()) {
		return "";
	} else {
		QUrl url(connectionString_);
		return url.fileName();
	}
}

bool WebDAVBackend::isOpen(void) const {
	return !stream_.isNull() && !connecting;
}

bool WebDAVBackend::openConnection(QString connectionString) {
	if (!canHandleUrl(connectionString)) {
		return false;
	}

	// prevent another connection attempt while already connecting
	// that's required here because ot the asynchronous character of
	// underlying tcp socket
	if (connecting) return false;

	// close connection in case we had one open
	if (isOpen()) closeConnection();

	QUrl url = QUrl::fromEncoded(connectionString.toLatin1());
	if (!url.isValid()) {
		return false;
	}

	QWebdav::QWebdavConnectionType connectionType;
	if (url.scheme() == "http") {
		connectionType = QWebdav::HTTP;
	} else if (url.scheme() == "https") {
		connectionType = QWebdav::HTTPS;
	} else {
		return false;
	}

	webdav.setConnectionSettings(connectionType, url.host(), url.path(), url.userName(), url.password(), url.port());
	webdav.setAlwaysIgnoreSslErrors(ignoreSslErrors);

	QNetworkReply* reply = webdav.get("");

	connect(reply, SIGNAL(readyRead()), this, SLOT(onDataReady()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyError(QNetworkReply::NetworkError)));

	stream_.reset(reply);
	connectionString_ = connectionString;

	connecting = true;
	return true;
}

void WebDAVBackend::closeConnection(void) {
	if (stream_.data() != nullptr) {
		QNetworkReply* reply = static_cast<QNetworkReply*>(stream_.data());

		if (!reply->isFinished()) {
			reply->abort();
		}
	}
	BaseBackend::closeConnection();
	stream_.reset();
}

void WebDAVBackend::onDataReady(void) {
	if (connecting) {
		connecting = false;
		emitConnected();
	}

	emitDataReady();
}

void WebDAVBackend::errorOccured(QString msg) {
	if (!connecting) {
		connectionWasLost();
	}

	Log::critical(msg);
}

void WebDAVBackend::replyError(QNetworkReply::NetworkError) {
	if (!connecting) {
		connectionWasLost();
	}

	QNetworkReply* reply = static_cast<QNetworkReply*>(stream_.data());

	Log::critical(reply->error() + ": " + reply->errorString());
}

