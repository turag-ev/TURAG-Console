#include "connectionwidgetwebdav.h"
#include <QSettings>
#include <QHostInfo>
#include <libs/log.h>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QUrl>
#include <qwebdav/qwebdavlib/qwebdavitem.h>
#include <QMessageBox>
#include <libs/logindialog.h>
#include <QSpacerItem>
#include <QGridLayout>
#include <QGroupBox>
#include <QTreeView>
#include <connectionwidgets/connectionwidgetwebdav/webdavtreeitem.h>
#include <QTableWidget>
#include <QHeaderView>

#ifdef QT_NO_OPENSSL
# error QT_NO_OPENSSL must not be defined
#endif

QIcon ConnectionWidgetWebDAV::unknownIcon(QIcon::fromTheme("unknown", QIcon(":/images/unknown.png")));


ConnectionWidgetWebDAV::ConnectionWidgetWebDAV (QWidget *parent) :
	ConnectionWidget("Letzte Verbindungen", parent), status(Status::unconnected), currentFileItem(nullptr)
{
	setObjectName("WebDAV");

	connect(&webdav, SIGNAL(errorChanged(QString)), this, SLOT(errorOccured(QString)));
	connect(&webdavDirParser, SIGNAL(finished()), this, SLOT(parsingFinished()));
	connect(&webdavDirParser, SIGNAL(errorChanged(QString)), this, SLOT(errorOccured(QString)));
	connect(&webdav, SIGNAL(checkSslCertifcate(QList<QSslError>)), this, SLOT(sslError(QList<QSslError>)));
	connect(&webdav, SIGNAL(authRequired()), this, SLOT(authRequired()));

    QSettings settings;
	settings.beginGroup("ConnectionWidgetWebDAV");
	QString recentHost = settings.value("host", QString("%1").arg(DEFAULTHOST)).toString();

    // Eingabemaske für Host
    // --------------------------------------------------
    QLabel * hostLabel = new QLabel("Host: ");

    //Eingabefeld für host
    hostEdit = new QLineEdit();
    hostEdit->setText(recentHost);
    connect(hostEdit, SIGNAL(returnPressed()), this, SLOT(connectToServer()));

    // create button to connect
    connect_button = new QPushButton("Verbinden");
    connect(connect_button, SIGNAL(clicked()), this, SLOT(connectToServer()));

    connect_cancel_button = new QPushButton("Abbrechen");
    connect(connect_cancel_button, SIGNAL(clicked()), this, SLOT(cancel_connecting()));

    QHBoxLayout * editLayout = new QHBoxLayout();
    editLayout->addWidget(hostLabel, 0);
    editLayout->addWidget(hostEdit, 1);
    editLayout->addWidget(connect_button, 0);
    editLayout->addWidget(connect_cancel_button, 0);
    editLayout->addStretch();

	// tree view
	// --------------------------------------------------
	view = new QTreeView;
	connect(view, SIGNAL(expanded(QModelIndex)), this, SLOT(expandItem(QModelIndex)));
	connect(view, SIGNAL(activated(QModelIndex)), this, SLOT(activateItem(QModelIndex)));

	fileList = new QTableWidget;
	fileList->setSelectionBehavior(QAbstractItemView::SelectRows);
	fileList->setSelectionMode(QAbstractItemView::SingleSelection);
	fileList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	fileList->setColumnCount(3);
	fileList->setHorizontalHeaderLabels(QStringList({"Name", "Size", "Modified"}));
	fileList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	fileList->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	fileList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	connect(fileList, SIGNAL(cellActivated(int,int)), this, SLOT(fileActivated(int,int)));

	QHBoxLayout* treeLayout = new QHBoxLayout;
	treeLayout->addWidget(view);
	treeLayout->addWidget(fileList);


    // add stuff to layout
    // ----------------------------------------
    layout->addLayout(editLayout);
	layout->addLayout(treeLayout);
	layout->setStretchFactor(treeLayout, 1);

    layout->addSpacing(10);
	addRecentConnections();
	enterUnconnectedState();
}

ConnectionWidgetWebDAV::~ConnectionWidgetWebDAV() {
    QSettings settings;
	settings.beginGroup("ConnectionWidgetWebDAV");

    settings.setValue("host", hostEdit->text());
}

/*
 *
 *SLOTS
 */
void ConnectionWidgetWebDAV::connectToServer() {
	QUrl url(QUrl::fromUserInput(hostEdit->text()));
	QWebdav::QWebdavConnectionType connectionType;

	if (!url.isValid()) {
		return;
	}

	if (url.scheme() == "http") {
		connectionType = QWebdav::HTTP;
	} else if (url.scheme() == "https") {
		connectionType = QWebdav::HTTPS;
	} else {
		return;
	}

	model.reset(new WebDAVTreeModel(QString("/")));
	view->setModel(model.data());
//	view->header()->setSectionResizeMode(0, QHeaderView::Stretch);
//	view->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

	currentlyQueriedItem = model->index(0, 0);

	webdav.setConnectionSettings(connectionType, url.host(), url.path(), QString(), QString(), url.port());
	webdavDirParser.listDirectory(&webdav, "/");
	enterConnectingState();
}

void ConnectionWidgetWebDAV::cancel_connecting(void) {
	webdavDirParser.abort();
	enterUnconnectedState();
}


void ConnectionWidgetWebDAV::parsingFinished(void) {
	switch (status) {
	case Status::error:
	case Status::unconnected:
		status = Status::unconnected;
		return;

	case Status::reconnecting:
		webdavDirParser.listDirectory(&webdav, "/");
		enterConnectingState();
		return;

	case Status::connecting: {
		saveConnection(hostEdit->text());
		addRecentConnections();

		parseData();

		enterConnectedState();
		return;
	}

	case Status::connected: {
		parseData();
		return;
	}
	default:
		return;
	}
}

void ConnectionWidgetWebDAV::parseData(void) {
	QList<WebDAVTreeItem*> itemList;
	WebDAVTreeItem* oldItem = static_cast<WebDAVTreeItem*>(currentlyQueriedItem.internalPointer());

	model->clear(currentlyQueriedItem);

	for (QWebdavItem& item : webdavDirParser.getList()) {
		WebDAVTreeItem* newItem;

		if (item.isDir()) {
			newItem = new WebDAVTreeItem(item.path(), item.name());
			itemList.append(newItem);
		} else {
			oldItem->addFile(item);
		}
	}
	model->addRows(itemList, currentlyQueriedItem);
	oldItem->setWasExpanded(true);

	fillFileList(currentlyQueriedItem);
}

void ConnectionWidgetWebDAV::errorOccured(QString errorMsg) {
	enterUnconnectedState(true);

	Log::critical(errorMsg);
}

void ConnectionWidgetWebDAV::sslError(const QList<QSslError> &errors) {
	QSslError error = errors.value(0);
	if (error.certificate().isNull()) {
		Log::critical(error.errorString());
		enterUnconnectedState(true);
		return;
	}

	QMessageBox dialog(this);
	dialog.setIcon(QMessageBox::Warning);
	dialog.setText("SSL error occured");
	dialog.setInformativeText(error.errorString());
	dialog.setDetailedText(error.certificate().toText());
	dialog.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	dialog.setDefaultButton(QMessageBox::Cancel);
	QSpacerItem* horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
	QGridLayout* layout = (QGridLayout*)dialog.layout();
	layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
	int result = dialog.exec();

	if (result == QMessageBox::Cancel)  {
		enterUnconnectedState();
		return;
	}

	QString sslCertDigestMd5 = QWebdav::digestToHex(error.certificate().digest(QCryptographicHash::Md5));
	QString sslCertDigestSha1 = QWebdav::digestToHex(error.certificate().digest(QCryptographicHash::Sha1));

	webdav.acceptSslCertificate(sslCertDigestMd5, sslCertDigestSha1);
	status = Status::reconnecting;
}

void ConnectionWidgetWebDAV::authRequired(void) {
	LoginDialog dialog;
	int result = dialog.exec();

	if (result == QDialog::Rejected)  {
		enterUnconnectedState();
		return;
	}

	webdav.setUsername(dialog.getUsername());
	webdav.setPassword(dialog.getPassword());
	status = Status::reconnecting;
}

void ConnectionWidgetWebDAV::onOpenRecentConnection(int index) {
	if (status == Status::unconnected) {
		hostEdit->setText(recent_connections.at(index));
		connectToServer();
	}
}

void ConnectionWidgetWebDAV::enterUnconnectedState(bool error) {
	if (!error) {
		status = Status::unconnected;
	} else {
		status = Status::error;
	}

	connect_button->setVisible(true);
	connect_cancel_button->setVisible(false);
	hostEdit->setEnabled(true);
	if (recentConnectionsContainer) recentConnectionsContainer->setEnabled(true);
	fileList->setEnabled(false);
	view->setEnabled(false);
}

void ConnectionWidgetWebDAV::enterConnectingState(void) {
	status = Status::connecting;

	connect_button->setVisible(false);
	connect_cancel_button->setVisible(true);
	connect_cancel_button->setText("Abbrechen");
	hostEdit->setEnabled(false);
	if (recentConnectionsContainer) recentConnectionsContainer->setEnabled(false);
	fileList->setEnabled(false);
	view->setEnabled(false);
}

void ConnectionWidgetWebDAV::enterConnectedState(void) {
	status = Status::connected;

	connect_button->setVisible(false);
	connect_cancel_button->setVisible(true);
	connect_cancel_button->setText("Trennen");
	hostEdit->setEnabled(false);
	if (recentConnectionsContainer) recentConnectionsContainer->setEnabled(false);
	fileList->setEnabled(true);
	fileList->clearContents();
	fileList->setRowCount(0);
	view->setEnabled(true);
}

void ConnectionWidgetWebDAV::expandItem(const QModelIndex & index) {
	if (index.isValid()) {
		currentlyQueriedItem = index;
		WebDAVTreeItem* item = static_cast<WebDAVTreeItem*>(index.internalPointer());

		webdavDirParser.listDirectory(&webdav, item->path());
	}
}

void ConnectionWidgetWebDAV::activateItem(const QModelIndex & index) {
	expandItem(index);

	fillFileList(index);
}

void ConnectionWidgetWebDAV::fillFileList(const QModelIndex & index) {
	if (index.isValid()) {
		currentFileItem = static_cast<WebDAVTreeItem*>(index.internalPointer());
		fileList->clearContents();
		fileList->setRowCount(currentFileItem->files().size());

		int row = 0;
		for (QWebdavItem& file : currentFileItem->files()) {
			QTableWidgetItem* nameEntry = new QTableWidgetItem(file.name());
			nameEntry->setIcon(unknownIcon);
			fileList->setItem(row, 0, nameEntry);
			QTableWidgetItem* sizeEntry = new QTableWidgetItem(formatFileSize(file.size()));
			fileList->setItem(row, 1, sizeEntry);
			QTableWidgetItem* modifiedEntry = new QTableWidgetItem(file.lastModifiedStr());
			fileList->setItem(row, 2, modifiedEntry);
			++row;
		}
		fileList->resizeRowsToContents();
	}
}

QString ConnectionWidgetWebDAV::formatFileSize(int size) {
	float num = static_cast<float>(size);
	QStringList list;
	list << "KB" << "MB" << "GB" << "TB";

	QStringListIterator i(list);
	QString unit("B");

	while(num >= 1024.0 && i.hasNext())
	 {
		unit = i.next();
		num /= 1024.0;
	}
	return QString().setNum(num,'f',2)+" "+unit;
}

void ConnectionWidgetWebDAV::fileActivated(int row,	int) {
	QTableWidgetItem* item = fileList->item(row, 0);

	if (currentFileItem && item) {
		QString fileName = item->text();

		QUrl url;
		if (webdav.isSSL()) {
			url.setScheme("https");
		} else {
			url.setScheme("http");
		}

		url.setHost(webdav.hostname());
		url.setPort(webdav.port());
		url.setUserName(webdav.username());
		url.setPassword(webdav.password());
		url.setPath(webdav.rootPath() + currentFileItem->path() + fileName);

		emit connectionChanged(url.toEncoded(), nullptr, nullptr);
	}


}
