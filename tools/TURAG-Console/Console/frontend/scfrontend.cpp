#include "scfrontend.h"
#include "logview.h"
#include "cmenufrontend.h"
#include <QHBoxLayout>
#include <QSplitter>
#include <QDebug>


SCFrontend::SCFrontend(QWidget *parent) :
    BaseFrontend("System Control", parent)
{
    logview = new LogView;
    cmenu = new CmenuFrontend;

    QHBoxLayout* layout = new QHBoxLayout;
    QSplitter* splitter = new QSplitter;

    layout->addWidget(splitter);
    splitter->addWidget(logview);
    splitter->addWidget(cmenu);

    setLayout(layout);

    // connect outputs of logview and cmenu to own dataReadySignal
    connect(logview, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
    connect(cmenu, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
}


void SCFrontend::writeData(QByteArray data) {
    logview->writeData(data);
    cmenu->writeData(data);
}

void SCFrontend::clear(void) {
    logview->clear();
    cmenu->clear();
}

bool SCFrontend::saveOutput(void) {
    return logview->saveOutput();

}

void SCFrontend::onConnected(bool readOnly, bool isSequential) {
    logview->onConnected(readOnly, isSequential);
    cmenu->onConnected(readOnly, isSequential);
    qDebug() << objectName() << " connected";
}

void SCFrontend::onDisconnected(bool reconnecting) {
    logview->onDisconnected(reconnecting);
    cmenu->onDisconnected(reconnecting);
    qDebug() << objectName() << " disconnected";
}
