#ifndef SCFRONTEND_H
#define SCFRONTEND_H

#include <tina++/tina.h>

#include "basefrontend.h"

class TinaInterface;
class PlainTextFrontend;
class LogView;

class SCFrontend : public BaseFrontend {
	Q_OBJECT

public:
    SCFrontend(QWidget *parent = 0);
    virtual void setExternalContextActions(QList<QAction*> actions);

public slots:
    void writeData(QByteArray data) override;
    void clear(void) override;
    bool saveOutput(void) override;

    // frontend was connected to stream
    void onConnected(bool readOnly, bool isBuffered, QIODevice*) override;

    // frontend was disconnected from stream
    void onDisconnected(bool reconnecting) override;

protected:
    TinaInterface* interface;
    LogView* logview;
    PlainTextFrontend* cmenu;

};

#endif // SCFRONTEND_H
