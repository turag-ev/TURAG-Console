#ifndef SCFRONTEND_H
#define SCFRONTEND_H

#include <QAbstractTableModel>
#include <tina++/tina.h>

#include "basefrontend.h"

class TinaInterface;
class PlainTextFrontend;
class LogView;

class SCFrontend : public BaseFrontend {
	Q_OBJECT

protected:
    TinaInterface* interface;
    LogView* logview;
    PlainTextFrontend* cmenu;

public:
    SCFrontend(QWidget *parent = 0);

public slots:
    void writeData(QByteArray data) override;
    void clear(void) override;
    bool saveOutput(void) override;

    // frontend was connected to stream
    void onConnected(bool readOnly, bool isSequential, QIODevice*) override;

    // frontend was disconnected from stream
    void onDisconnected(bool reconnecting) override;
};

#endif // SCFRONTEND_H
