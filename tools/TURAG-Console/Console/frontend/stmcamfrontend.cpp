#include "stmcamfrontend.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QDebug>

#include "logview.h"
#include "plaintextfrontend.h"
#include "util/tinainterface.h"

////////////////////////////////////////////////////////////////////////////////
// STMCamFrontend

STMCamFrontend::STMCamFrontend(QWidget *parent) :
    BaseFrontend("STMCam", parent)
{
    interface = new TinaInterface(this);
    logview = new LogView(interface);
    cshell = new PlainTextFrontend();

    QHBoxLayout* layout = new QHBoxLayout;
    QSplitter* splitter = new QSplitter;

    layout->addWidget(splitter);
    layout->setMargin(0);
    splitter->addWidget(logview);
    splitter->addWidget(cshell);
    splitter->setContentsMargins(0,0,0,0);
    setLayout(layout);

    connect(interface, SIGNAL(cmenuDataReady(QByteArray)), cshell, SLOT(writeData(QByteArray)));

    // connect outputs of logview and cmenu to own dataReadySignal
    connect(logview, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
    connect(cshell, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
}

void STMCamFrontend::writeData(QByteArray data) {
    interface->dataInput(data);
}

void STMCamFrontend::clear(void) {
    logview->clear();
    cshell->clear();
    interface->clear();
}

bool STMCamFrontend::saveOutput(QString file) {
    return logview->saveOutput(file);
}

void STMCamFrontend::onConnected(bool readOnly, bool isBuffered, QIODevice* dev) {
    Q_UNUSED(dev);

    logview->onConnected(readOnly, isBuffered, dev);
    cshell->onConnected(readOnly, isBuffered, dev);
    qDebug() << objectName() << " connected";
}

void STMCamFrontend::onDisconnected(bool reconnecting) {
    logview->onDisconnected(reconnecting);
    cshell->onDisconnected(reconnecting);
    qDebug() << objectName() << " disconnected";
}

void STMCamFrontend::setExternalContextActions(QList<QAction*> actions) {
    cshell->setExternalContextActions(actions);
    BaseFrontend::setExternalContextActions(actions);
}
