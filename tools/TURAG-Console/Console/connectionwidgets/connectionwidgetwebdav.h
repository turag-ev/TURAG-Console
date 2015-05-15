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
#include <QScopedPointer>
#include <connectionwidgets/connectionwidgetwebdav/webdavtreemodel.h>
#include <QModelIndex>

class QTreeView;
class QTableWidget;

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
	void expandItem(const QModelIndex & index);
	void activateItem(const QModelIndex & index);
	void fileActivated(int row,	int col);

private:
	enum class Status { unconnected, connecting, error, connected, reconnecting };

	void enterConnectingState(void);
	void enterConnectedState(void);
	void enterUnconnectedState(bool error = false);
	void parseData(void);
	QString formatFileSize(int size);
	void fillFileList(const QModelIndex & index);

    QLineEdit * hostEdit;
    QPushButton * connect_button;
    QPushButton * connect_cancel_button;
	QTreeView* view;
	QTableWidget* fileList;

	QWebdav webdav;
	QWebdavDirParser webdavDirParser;
	bool connectionFailed;
	Status status;

	QScopedPointer<WebDAVTreeModel> model;
	QModelIndex currentlyQueriedItem;
	WebDAVTreeItem* currentFileItem;

	static QIcon unknownIcon;
};

#endif // CONNECTIONWIDGETWEBDAV_H
