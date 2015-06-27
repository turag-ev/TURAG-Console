#ifndef MCFRONTEND_H
#define MCFRONTEND_H

#include "basefrontend.h"

class TinaInterface;
class PlainTextFrontend;
class RobotLogFrontend;
class QStackedWidget;
class TinaGraphFrontend;
class QTabWidget;


class RobotFrontend : public BaseFrontend
{
    Q_OBJECT
public:
	explicit RobotFrontend(QWidget *parent = 0);

public slots:
    void writeData(QByteArray data) override;
    void clear(void) override;

    // frontend was connected to stream
	void onConnected(bool readOnly, QIODevice*) override;

    // frontend was disconnected from stream
    void onDisconnected(bool reconnecting) override;

private slots:
    void activatedMessage(char, QString message);

private:
    TinaInterface* interface;
    RobotLogFrontend* logview;
    PlainTextFrontend* cmenu;
    TinaGraphFrontend* graphView;
    QTabWidget* tabs;
};

#endif // MCFRONTEND_H
