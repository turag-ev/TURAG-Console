#ifndef CONNECTIONWIDGETWEBDAV_H
#define CONNECTIONWIDGETWEBDAV_H

#include "connectionwidget.h"

#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <qwebdav/qwebdavlib/qwebdav.h>
#include <qwebdav/qwebdavlib/qwebdavdirparser.h>
#include <QList>
#include <QSslError>

class QTreeView;

class ConnectionWidgetWebDAV: public ConnectionWidget {
    Q_OBJECT

public:
	static constexpr const char* DEFAULTHOST = "http://foxie/webdav/public";

	ConnectionWidgetWebDAV(QWidget *parent = 0);
	~ConnectionWidgetWebDAV();

public slots:

protected:

protected slots:
    virtual void onOpenRecentConnection(int index);

private slots:
	void connectToServer();
	void cancel_connecting(void);
	void parsingFinished(void);
	void errorOccured(QString msg);
	void sslError(const QList<QSslError> &errors);
	void authRequired(void);

private:
	enum class Status { unconnected, connecting, error, connected, reconnecting };

	void enterConnectingState(void);
	void enterConnectedState(void);
	void enterUnconnectedState(bool error = false);

    QLineEdit * hostEdit;
    QPushButton * connect_button;
    QPushButton * connect_cancel_button;
	QTreeView* view;

	QWebdav webdav;
	QWebdavDirParser webdavDirParser;
	bool connectionFailed;
	Status status;
};

#endif // CONNECTIONWIDGETWEBDAV_H
