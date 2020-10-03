#ifndef TURAG_CONSOLE_CONSOLE_BACKEND_WEBDAVBACKEND_H
#define TURAG_CONSOLE_CONSOLE_BACKEND_WEBDAVBACKEND_H

#include "basebackend.h"
#include <qt/qwebdav/qwebdavlib/qwebdav.h>


class WebDAVBackend : public BaseBackend
{
	Q_OBJECT
public:
	WebDAVBackend(bool ignoreSslErrors = false, QObject* parent = nullptr);
	virtual ~WebDAVBackend();

	virtual QString getConnectionInfo() const;
	virtual bool isOpen(void) const;


private slots:
	void errorOccured(QString msg);
	void replyError(QNetworkReply::NetworkError);
	void onDataReady(void);
	void finished(void);


private:
	enum class State { unconnected, connecting, error, connected, reconnecting };

	virtual bool doConnectionPreconditionChecking(const QUrl& url);
	virtual BaseBackend::ConnectionStatus doOpenConnection(const QUrl& connectionUrl);
	virtual void doCleanUpConnection(void);

	bool ignoreSslErrors;
	QWebdav webdav;
	State state;
};

#endif // TURAG_CONSOLE_CONSOLE_BACKEND_WEBDAVBACKEND_H
