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

void TinaCameraFrontend::onConnected(bool , bool, QIODevice*) {
    
}

void TinaCameraFrontend::onDisconnected(bool ) {
    
}

void TinaCameraFrontend::clear(void) {

}

bool TinaCameraFrontend::saveOutput(QString file) {
    (void)file;
    return false;
}

// needed for the interface
void TinaCameraFrontend::writeData(QByteArray data) {
    (void)data;
}
