#include "mcfrontend.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QDebug>
#include <QStackedWidget>

#include "logview.h"
#include "plaintextfrontend.h"
#include "util/tinainterface.h"
#include "tinagraphfrontend.h"


MCFrontend::MCFrontend(QWidget *parent) :
    BaseFrontend("Motion Control", parent)
{
    interface = new TinaInterface(this);
    logview = new LogView(interface);
    cmenu = new PlainTextFrontend();
    graphView = new TinaGraphFrontend;

    QHBoxLayout* layout = new QHBoxLayout;
    QSplitter* splitter = new QSplitter;
    QSplitter* vsplitter = new QSplitter;

    layout->addWidget(splitter);
    layout->setMargin(0);
    setLayout(layout);
    splitter->addWidget(logview);
    splitter->addWidget(vsplitter);
    splitter->setContentsMargins(0,0,0,0);

    vsplitter->setOrientation(Qt::Vertical);
    vsplitter->addWidget(cmenu);
    vsplitter->addWidget(graphView);
    vsplitter->setContentsMargins(0,0,0,0);


    connect(interface, SIGNAL(cmenuDataReady(QByteArray)), cmenu, SLOT(writeData(QByteArray)));

    // connect outputs of logview and cmenu to own dataReadySignal
    connect(logview, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
    connect(cmenu, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
    connect(logview, SIGNAL(activatedGraph(int)), graphView, SLOT(activateGraph(int)));
    connect(interface, SIGNAL(tinaPackageReady(QByteArray)), graphView, SLOT(writeLine(QByteArray)));

}


void MCFrontend::writeData(QByteArray data) {
    interface->dataInput(data);
}

void MCFrontend::clear(void) {
    logview->clear();
    cmenu->clear();
    graphView->clear();
    interface->clear();
}

bool MCFrontend::saveOutput(void) {
    return logview->saveOutput();
}

void MCFrontend::onConnected(bool readOnly, bool isSequential, QIODevice* dev) {
    Q_UNUSED(dev);

    logview->onConnected(readOnly, isSequential, dev);
    cmenu->onConnected(readOnly, isSequential, dev);
    graphView->onConnected(readOnly, isSequential, dev);
    qDebug() << objectName() << " connected";
}

void MCFrontend::onDisconnected(bool reconnecting) {
    logview->onDisconnected(reconnecting);
    cmenu->onDisconnected(reconnecting);
    graphView->onDisconnected(reconnecting);
    qDebug() << objectName() << " disconnected";
}

void MCFrontend::setExternalContextActions(QList<QAction*> actions) {
    cmenu->setExternalContextActions(actions);
    graphView->setExternalContextActions(actions);
    BaseFrontend::setExternalContextActions(actions);
}
