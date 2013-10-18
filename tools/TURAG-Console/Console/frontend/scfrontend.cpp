#include "scfrontend.h"
#include <QHBoxLayout>
#include <QSplitter>


SCFrontend::SCFrontend(QWidget *parent) :
    BaseFrontend("System Control", parent)
{
    logview = new LogView;
    commandview = new CommandView;

    QHBoxLayout* layout = new QHBoxLayout;
    QSplitter* splitter = new QSplitter;

    layout->addWidget(splitter);
    splitter->addWidget(logview);
    splitter->addWidget(commandview);

    setLayout(layout);

    connect(logview, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
    connect(commandview, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
}


void SCFrontend::writeData(QByteArray data) {
    logview->writeData(data);
    commandview->writeData(data);
}

void SCFrontend::clear(void) {
    logview->clear();
    commandview->clear();
}

bool SCFrontend::saveOutput(void) {
    logview->saveOutput();
}

void SCFrontend::onConnected(bool readOnly, bool isSequential) {
    logview->onConnected(readOnly, isSequential);
    commandview->onConnected(readOnly, isSequential);
}

void SCFrontend::onDisconnected(bool reconnecting) {
    logview->onDisconnected(reconnecting);
    commandview->onDisconnected(reconnecting);
}
