#ifndef STMCAMFRONTEND_H
#define STMCAMFRONTEND_H

#include <tina++/tina.h>

#include "basefrontend.h"

class TinaInterface;
class TinaCameraFrontend;
class RobotLogFrontend;
class PlainTextFrontend;

class STMCamFrontend : public BaseFrontend {
    Q_OBJECT

public:
    STMCamFrontend(QWidget *parent = 0);

public slots:
    void writeData(QByteArray data) override;
    void clear(void) override;

    // frontend was connected to stream
	void onConnected(bool readOnly, QIODevice*) override;

    // frontend was disconnected from stream
    void onDisconnected(bool reconnecting) override;

protected:
    TinaInterface* interface;
    TinaCameraFrontend* camview;
    RobotLogFrontend* logview;
    PlainTextFrontend* cshell;

};

#endif // STMCAMFRONTEND_H
