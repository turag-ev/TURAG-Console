#include "odocalfrontend.h"

#include <QHBoxLayout>
#include <QSplitter>

#include "libs/iconmanager.h"
#include "libs/splitterext.h"

#include "util/tinainterface.h"

#include "logview.h"
#include "plaintextfrontend.h"

OdocalFrontend::OdocalFrontend(QWidget *parent) :
    BaseFrontend("Odometrie-Kalibrierung", IconManager::get("turag-tina"), parent)
{
    tinaInterface = new TinaInterface(this);
    logview = new RobotLogFrontend(tinaInterface);
    cmenu = new PlainTextFrontend();

    QHBoxLayout* layout = new QHBoxLayout;
    SplitterExt* splitter = new SplitterExt("robotFrontend_splitter");

    /*
     * Add Odocal Menu Buttons:
     * - Last Parameters (List)
     * - Input current parameters manually
     * - After each iteration, copy new parameters to
     *   current parameter
     * - Insert parameter from list?
     * - Warn if no calibrate mode
     * - Input fields for bot geometry
     * - image for bot geometry?
     * - input field for current offset
     */

    layout->addWidget(splitter);
    setLayout(layout);
    splitter->addWidget(logview);
    splitter->addWidget(cmenu);
    splitter->restoreState();

    connect(tinaInterface, SIGNAL(cmenuDataReady(QByteArray)), cmenu, SLOT(writeData(QByteArray)));

    // connect outputs of logview and cmenu to own dataReadySignal
    connect(logview, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
    connect(cmenu, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
    connect(logview, SIGNAL(activatedMessage(char,QString)), this, SLOT(activatedMessage(char,QString)));
    //connect(tinaInterface, SIGNAL(tinaPackageReady(QByteArray)), graphView, SLOT(writeLine(QByteArray)));
}

void OdocalFrontend::writeData(QByteArray data) {
    tinaInterface->dataInput(data);
}

void OdocalFrontend::clear(void) {
    logview->clear();
    cmenu->clear();
    tinaInterface->clear();
}

void OdocalFrontend::onConnected(bool readOnly, QIODevice* dev) {
    logview->onConnected(readOnly, dev);
    cmenu->onConnected(readOnly, dev);
}

void OdocalFrontend::onDisconnected(bool reconnecting) {
    logview->onDisconnected(reconnecting);
    cmenu->onDisconnected(reconnecting);
}


void OdocalFrontend::activatedMessage(char,QString message)
{
    if (message.startsWith(QStringLiteral("Graph")) && message.size() > 6)
    {
        bool ok = false;
        int i = message.mid(6, message.indexOf(':') - 6).toInt(&ok);
        if (ok) {
            //tabs->setCurrentIndex(1);
            //graphView->activateGraph(i);
        }
    }
}
