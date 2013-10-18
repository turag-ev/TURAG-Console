#ifndef SCFRONTEND_H
#define SCFRONTEND_H

#include "basefrontend.h"
#include "logview.h"
#include "commandview.h"


class SCFrontend : public BaseFrontend
{
protected:
    LogView* logview;
    CommandView* commandview;

public:
    SCFrontend(QWidget *parent = 0);

public slots:
    virtual void writeData(QByteArray data);
    virtual void clear(void);
    virtual bool saveOutput(void);

    // frontend was connected to stream, does nothing if not overloaded
    virtual void onConnected(bool readOnly, bool isSequential);

    // frontend was disconnected from stream, does nothing if not overloaded
    virtual void onDisconnected(bool reconnecting);
};

#endif // SCFRONTEND_H
