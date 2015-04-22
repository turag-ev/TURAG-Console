#include "robotfrontend.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QDebug>
#include <QTabWidget>

#include "logview.h"
#include "plaintextfrontend.h"
#include "util/tinainterface.h"
#include "tinagraphfrontend.h"


RobotFrontend::RobotFrontend(QWidget *parent) :
	BaseFrontend("TURAG Roboter-Ansicht (TinA)", parent)
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
    //connect(graphView, SIGNAL(newGraph(int)), this, SLOT(activateGraph(int)));

}


void RobotFrontend::writeData(QByteArray data) {
    interface->dataInput(data);
}

void RobotFrontend::clear(void) {
    logview->clear();
    cmenu->clear();
    graphView->clear();
    interface->clear();
}

void RobotFrontend::onConnected(bool readOnly, bool isBuffered, QIODevice* dev) {
    logview->onConnected(readOnly, isBuffered, dev);
    cmenu->onConnected(readOnly, isBuffered, dev);
    graphView->onConnected(readOnly, isBuffered, dev);
}

void RobotFrontend::onDisconnected(bool reconnecting) {
    logview->onDisconnected(reconnecting);
    cmenu->onDisconnected(reconnecting);
    graphView->onDisconnected(reconnecting);
}


void RobotFrontend::activateGraph(int index) {
    tabs->setCurrentIndex(1);
    graphView->activateGraph(index);

}
