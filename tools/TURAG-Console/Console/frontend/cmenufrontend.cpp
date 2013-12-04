
#include "cmenufrontend.h"
#include "util/tinainterface.h"
#include <QByteArray>
#include <QWidget>

/*
CmenuFrontend::CmenuFrontend(TinaInterface *interface, QWidget *parent) :
    PlainTextFrontend(parent), dataInterface(interface)
{
    connect(dataInterface, SIGNAL(cmenuDataReady(QByteArray)), this, SLOT(writeDataBase(QByteArray)));
}

void CmenuFrontend::writeData(QByteArray data) {
    dataInterface->dataInput(data);
}


void CmenuFrontend::writeDataBase(QByteArray data) {
    PlainTextFrontend::writeData(data);
}*/
