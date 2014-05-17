#include "stmcamfrontend.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QDebug>

#include "tinacamerafrontend.h"
#include "logview.h"
#include "plaintextfrontend.h"
#include "util/tinainterface.h"

////////////////////////////////////////////////////////////////////////////////
// STMCamFrontend

STMCamFrontend::STMCamFrontend(QWidget *parent) :
    BaseFrontend("STMCam", parent)
{
    interface = new TinaInterface(this);
    camview = new TinaCameraFrontend(this);
    logview = new LogView(interface);
    cshell = new PlainTextFrontend(this);

    // IO box
    QHBoxLayout* IOlayout = new QHBoxLayout;
    QSplitter* IOsplitter = new QSplitter;

    IOlayout->addWidget(IOsplitter);
    IOlayout->setMargin(0);

    // top: logview, bottom: shell
    IOsplitter->setOrientation(Qt::Vertical);
    IOsplitter->addWidget(logview);
    IOsplitter->addWidget(cshell);
    IOsplitter->setContentsMargins(0,0,0,0);

    // main box
    QHBoxLayout* viewlayout = new QHBoxLayout;
    QSplitter* viewsplitter = new QSplitter;

    viewlayout->addWidget(viewsplitter);
    viewlayout->setMargin(0);

    // left: cam viewer, right: IO box
    viewsplitter->addWidget(camview);
    viewsplitter->addWidget(IOsplitter);
    viewsplitter->setContentsMargins(0,0,0,0);

    // set main box as layout
    setLayout(viewlayout);

    connect(interface, SIGNAL(cmenuDataReady(QByteArray)), cshell, SLOT(writeData(QByteArray)));
    connect(interface, SIGNAL(tinaPackageReady(QByteArray)), camview, SLOT(writeLine(QByteArray)));

    // connect outputs of logview and cmenu to own dataReadySignal
    connect(camview, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
    connect(logview, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
    connect(cshell, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
}

void STMCamFrontend::writeData(QByteArray data) {
    interface->dataInput(data);
}

void STMCamFrontend::clear(void) {
    logview->clear();
    cshell->clear();
    camview->clear();
    interface->clear();
}

void STMCamFrontend::onConnected(bool readOnly, bool isBuffered, QIODevice* dev) {
    Q_UNUSED(dev);

    logview->onConnected(readOnly, isBuffered, dev);
    cshell->onConnected(readOnly, isBuffered, dev);
    camview->onConnected(readOnly, isBuffered, dev);
    qDebug() << objectName() << " connected";
}

void STMCamFrontend::onDisconnected(bool reconnecting) {
    logview->onDisconnected(reconnecting);
    cshell->onDisconnected(reconnecting);
    camview->onDisconnected(reconnecting);
    qDebug() << objectName() << " disconnected";
}

void STMCamFrontend::setExternalContextActions(QList<QAction*> actions) {
    cshell->setExternalContextActions(actions);
    camview->setExternalContextActions(actions);
    BaseFrontend::setExternalContextActions(actions);
}
