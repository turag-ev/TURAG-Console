#ifndef MCFRONTEND_H
#define MCFRONTEND_H

#include "basefrontend.h"

class TinaInterface;
class PlainTextFrontend;
class LogView;
class QStackedWidget;
class TinaGraphFrontend;
class QTabWidget;


class MCFrontend : public BaseFrontend
{
    Q_OBJECT
public:
    explicit MCFrontend(QWidget *parent = 0);
    virtual void setExternalContextActions(QList<QAction*> actions);

signals:

public slots:
    void writeData(QByteArray data) override;
    void clear(void) override;

    // frontend was connected to stream
    void onConnected(bool readOnly, bool isBuffered, QIODevice*) override;

    // frontend was disconnected from stream
    void onDisconnected(bool reconnecting) override;

private slots:
    void activateGraph(int index);

private:
    TinaInterface* interface;
    LogView* logview;
    PlainTextFrontend* cmenu;
    TinaGraphFrontend* graphView;
    QTabWidget* tabs;

};

#endif // MCFRONTEND_H
