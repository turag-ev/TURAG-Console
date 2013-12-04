#ifndef SCFRONTEND_H
#define SCFRONTEND_H

#include "basefrontend.h"

class TinaInterface;
class PlainTextFrontend;
class LogView;

class SCFrontend : public BaseFrontend
{
protected:
    TinaInterface* interface;
    LogView* logview;
    PlainTextFrontend* cmenu;

public:
    SCFrontend(QWidget *parent = 0);

public slots:
    virtual void writeData(QByteArray data);
    virtual void clear(void);
    virtual bool saveOutput(void);

    // frontend was connected to stream
    virtual void onConnected(bool readOnly, bool isSequential);

    // frontend was disconnected from stream
    virtual void onDisconnected(bool reconnecting);
};

#endif // SCFRONTEND_H
