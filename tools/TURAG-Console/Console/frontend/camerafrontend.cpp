#include "camerafrontend.h"
#include "util/datagraph.h"
#include <tina++/utils/base64.h>
#include <QHBoxLayout>
#include <QListWidget>
#include <QCamera>
#include <QCameraViewfinder>
#include <QVideoWidget>

TinaCameraFrontend::TinaCameraFrontend(QWidget *parent) :
    BaseFrontend("TinaCameraFrontend", parent)
{
    QHBoxLayout* layout = new QHBoxLayout;

    QByteArray cameraDevice;

    foreach (const QByteArray &deviceName, QCamera::availableDevices()) {
        cameraDevice = deviceName;
    }

    QCamera* camera = new QCamera(cameraDevice);

    QCameraViewfinder* viewfinder = new QCameraViewfinder;
    camera->setViewfinder(viewfinder);
    camera->setCaptureMode(QCamera::CaptureStillImage);
    layout->addWidget(viewfinder);

    setLayout(layout);

    camera->start();
}

void TinaCameraFrontend::writeLine(QByteArray line) {
    if (line.size() > 2) {
        char level = line.at(1);
        unsigned type = line.at(0);
        line.remove(0, 2);

        if (level == 'B') {
            bool ok = false;
            int space_pos = line.indexOf(' ');
            int index = line.left(space_pos).toInt(&ok);
            (void)index;

            if (ok) {
                switch (type) {
                case ':': {
                    // TODO
                    qDebug() << "image data arrived!";
                    break;
                }
                }
            }
        }
    }
}


void TinaCameraFrontend::activateGraph(int index) {
    int listindex = graphIndices.indexOf(index);
    if (listindex != -1) {
        activateGraphInternal(listindex);
    }
}

void TinaCameraFrontend::activateGraphInternal(int index) {
    stack->setCurrentIndex(index);
    graphlist->setCurrentRow(index);
    DataGraph* graph = static_cast<DataGraph*>(stack->currentWidget());
    clearActions();
    if (graph) addActions(graph->getActions());
}

void TinaCameraFrontend::onConnected(bool , bool, QIODevice*) {
    
}


void TinaCameraFrontend::onDisconnected(bool ) {
    
}

void TinaCameraFrontend::clear(void) {
    QWidget* widget;
    while ((widget = stack->currentWidget())) {
        stack->removeWidget(widget);
        widget->deleteLater();
    }
    graphIndices.clear();
    graphlist->clear();
}

bool TinaCameraFrontend::saveOutput(QString file) {
    if (stack->count()) {
        DataGraph* graph = static_cast<DataGraph*>(stack->currentWidget());
        return graph->saveOutput(file);
    } else {
        return false;
    }
}

// needed for the interface
void TinaCameraFrontend::writeData(QByteArray data) {
    (void)data;
}


