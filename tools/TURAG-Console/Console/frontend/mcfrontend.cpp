#include "mcfrontend.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QDebug>
#include <QTabWidget>

#include "logview.h"
#include "plaintextfrontend.h"
#include "util/tinainterface.h"
#include "tinagraphfrontend.h"


MCFrontend::MCFrontend(QWidget *parent) :
    BaseFrontend("Motion Control", parent)
{
    tabs = new QTabWidget;
    interface = new TinaInterface(this);
    logview = new LogView(interface);
    cmenu = new PlainTextFrontend();
    graphView = new TinaGraphFrontend;
    graphView->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);


    QHBoxLayout* layout = new QHBoxLayout;
    QSplitter* splitter = new QSplitter;

    layout->addWidget(tabs);
//    layout->setMargin(0);
    setLayout(layout);
    tabs->addTab(splitter, "Log");
    splitter->addWidget(logview);
    splitter->addWidget(cmenu);
//    splitter->setContentsMargins(0,0,0,0);

    tabs->addTab(graphView, "Diagramme");


    connect(interface, SIGNAL(cmenuDataReady(QByteArray)), cmenu, SLOT(writeData(QByteArray)));

    // connect outputs of logview and cmenu to own dataReadySignal
    connect(logview, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
    connect(cmenu, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
    connect(logview, SIGNAL(activatedGraph(int)), this, SLOT(activateGraph(int)));
    connect(interface, SIGNAL(tinaPackageReady(QByteArray)), graphView, SLOT(writeLine(QByteArray)));
    connect(graphView, SIGNAL(newGraph(int)), this, SLOT(activateGraph(int)));

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

bool MCFrontend::saveOutput(QString file) {
    return logview->saveOutput(file);
}

void MCFrontend::onConnected(bool readOnly, bool isBuffered, QIODevice* dev) {
    Q_UNUSED(dev);

    logview->onConnected(readOnly, isBuffered, dev);
    cmenu->onConnected(readOnly, isBuffered, dev);
    graphView->onConnected(readOnly, isBuffered, dev);
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


void MCFrontend::activateGraph(int index) {
    tabs->setCurrentIndex(1);
    graphView->activateGraph(index);

}
