#include "scfrontend.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QDebug>

#include "logview.h"
#include "plaintextfrontend.h"
#include "util/tinainterface.h"

////////////////////////////////////////////////////////////////////////////////
// SCFrontend

SCFrontend::SCFrontend(QWidget *parent) :
    BaseFrontend("System Control", parent)
{
    interface = new TinaInterface(this);
    logview = new LogView(interface);
    cmenu = new PlainTextFrontend();

    QHBoxLayout* layout = new QHBoxLayout;
    QSplitter* splitter = new QSplitter;

    layout->addWidget(splitter);
	layout->setMargin(0);
    splitter->addWidget(logview);
	splitter->addWidget(cmenu);
	splitter->setContentsMargins(0,0,0,0);
    setLayout(layout);

    connect(interface, SIGNAL(cmenuDataReady(QByteArray)), cmenu, SLOT(writeData(QByteArray)));

    // connect outputs of logview and cmenu to own dataReadySignal
    connect(logview, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
    connect(cmenu, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
}


void SCFrontend::writeData(QByteArray data) {
    interface->dataInput(data);
}

void SCFrontend::clear(void) {
    logview->clear();
    cmenu->clear();
}

bool SCFrontend::saveOutput(void) {
	return logview->saveOutput();
}

void SCFrontend::onConnected(bool readOnly, bool isSequential, QIODevice* dev) {
	Q_UNUSED(dev);

    logview->onConnected(readOnly, isSequential, dev);
    cmenu->onConnected(readOnly, isSequential, dev);
    qDebug() << objectName() << " connected";
}

void SCFrontend::onDisconnected(bool reconnecting) {
    logview->onDisconnected(reconnecting);
    cmenu->onDisconnected(reconnecting);
    qDebug() << objectName() << " disconnected";
}

void SCFrontend::setExternalContextActions(QList<QAction*> actions) {
    cmenu->setExternalContextActions(actions);
    BaseFrontend::setExternalContextActions(actions);
}
