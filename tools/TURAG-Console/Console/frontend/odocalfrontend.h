#ifndef ODOCALFRONTEND_H
#define ODOCALFRONTEND_H

#include "basefrontend.h"

class TinaInterface;
class PlainTextFrontend;
class RobotLogFrontend;
class QStackedWidget;
class QTabWidget;


class OdocalFrontend : public BaseFrontend
{
    Q_OBJECT
public:
    explicit OdocalFrontend(QWidget *parent = 0);

public slots:
    void writeData(QByteArray data) override;
    void clear(void) override;

    void onConnected(bool readOnly, QIODevice*) override;
    void onDisconnected(bool reconnecting) override;

private slots:
    void activatedMessage(char, QString message);

private:
    TinaInterface* tinaInterface;
    RobotLogFrontend* logview;
    PlainTextFrontend* cmenu;
};

#endif // ODOCALFRONTEND_H
