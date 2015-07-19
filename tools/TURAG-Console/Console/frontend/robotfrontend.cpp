#include "robotfrontend.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QTabWidget>

#include "logview.h"
#include "plaintextfrontend.h"
#include "util/tinainterface.h"
#include "tinagraphfrontend.h"
#include <libs/iconmanager.h>


RobotFrontend::RobotFrontend(QWidget *parent) :
	BaseFrontend("TURAG Roboter-Ansicht (TinA)", IconManager::get("turag-tina"), parent)
{
    tabs = new QTabWidget;
    interface = new TinaInterface(this);
    logview = new RobotLogFrontend(interface);
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
    connect(logview, SIGNAL(activatedMessage(char,QString)), this, SLOT(activatedMessage(char,QString)));
    connect(interface, SIGNAL(tinaPackageReady(QByteArray)), graphView, SLOT(writeLine(QByteArray)));
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

void RobotFrontend::onConnected(bool readOnly, QIODevice* dev) {
	logview->onConnected(readOnly, dev);
	cmenu->onConnected(readOnly, dev);
	graphView->onConnected(readOnly, dev);
}

void RobotFrontend::onDisconnected(bool reconnecting) {
    logview->onDisconnected(reconnecting);
    cmenu->onDisconnected(reconnecting);
    graphView->onDisconnected(reconnecting);
}


void RobotFrontend::activatedMessage(char,QString message)
{
	if (message.startsWith(QStringLiteral("Graph")) && message.size() > 6)
	{
		bool ok = false;
		int i = message.mid(6, message.indexOf(':') - 6).toInt(&ok);
		if (ok) {
			tabs->setCurrentIndex(1);
		    graphView->activateGraph(i);
		}
	}
}
