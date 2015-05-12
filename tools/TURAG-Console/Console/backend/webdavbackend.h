#ifndef WEBDAVBACKEND_H
#define WEBDAVBACKEND_H

#include "basebackend.h"
#include <qwebdav/qwebdavlib/qwebdav.h>


class WebDAVBackend : public BaseBackend
{
	Q_OBJECT
public:
	WebDAVBackend(bool ignoreSslErrors = false, QObject* parent = nullptr);
	virtual ~WebDAVBackend() {
		closeConnection();
	}

	virtual QString getConnectionInfo();
	virtual bool isOpen(void) const;

public slots:
	virtual bool openConnection(QString connectionString);
	virtual void closeConnection(void);

private slots:
	void errorOccured(QString msg);
	void replyError(QNetworkReply::NetworkError);
	void onDataReady(void);


private:
	enum class State { unconnected, connecting, error, connected, reconnecting };

	bool ignoreSslErrors;
	QWebdav webdav;
	State state;
	bool connecting;
};

#endif // WEBDAVBACKEND_H
