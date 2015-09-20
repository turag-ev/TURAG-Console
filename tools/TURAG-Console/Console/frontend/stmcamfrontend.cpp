#include "stmcamfrontend.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QDebug>

#include "tinacamerafrontend.h"
#include "logview.h"
#include "plaintextfrontend.h"
#include "util/tinainterface.h"
#include <libs/iconmanager.h>

////////////////////////////////////////////////////////////////////////////////
// STMCamFrontend

STMCamFrontend::STMCamFrontend(QWidget *parent) :
	BaseFrontend("STMCam", IconManager::get("camera-web"), parent)
{
    tinaInterface = new TinaInterface(this);
	camview = new TinaCameraFrontend(this);
    logview = new RobotLogFrontend(tinaInterface);
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

    connect(tinaInterface, SIGNAL(cmenuDataReady(QByteArray)), cshell, SLOT(writeData(QByteArray)));
    connect(tinaInterface, SIGNAL(tinaPackageReady(QByteArray)), camview, SLOT(writeLine(QByteArray)));

    // connect outputs of logview and cmenu to own dataReadySignal
    connect(camview, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
    connect(logview, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
    connect(cshell, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
}

void STMCamFrontend::writeData(QByteArray data) {
    tinaInterface->dataInput(data);
}

void STMCamFrontend::clear(void) {
    logview->clear();
    cshell->clear();
    camview->clear();
    tinaInterface->clear();
}

void STMCamFrontend::onConnected(bool readOnly, QIODevice* dev) {
	logview->onConnected(readOnly, dev);
	cshell->onConnected(readOnly, dev);
	camview->onConnected(readOnly, dev);
}

void STMCamFrontend::onDisconnected(bool reconnecting) {
    logview->onDisconnected(reconnecting);
    cshell->onDisconnected(reconnecting);
    camview->onDisconnected(reconnecting);
}

