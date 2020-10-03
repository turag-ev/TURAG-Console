#ifndef TURAG_CONSOLE_CONSOLE_FRONTEND_TINACAMERAFRONTEND_H
#define TURAG_CONSOLE_CONSOLE_FRONTEND_TINACAMERAFRONTEND_H

#include <QStackedWidget>
#include <QList>
#include <QGraphicsScene>
#include "basefrontend.h"

class QAction;
class QListWidget;

class TinaCameraFrontend : public BaseFrontend
{
    Q_OBJECT
public:
    explicit TinaCameraFrontend(QWidget *parent = 0);

public slots:
    void writeLine(QByteArray line);
    virtual void writeData(QByteArray data);

    // frontend was connected to stream
	void onConnected(bool readOnly, QIODevice*);

    // frontend was disconnected from stream
    void onDisconnected(bool reconnecting);
    void clear(void);

    // ui handler
    void handleButtonDumpOne(void);
    void handleButtonDumpAll(bool);
    void handleButtonFarbsensorInfo(void);
    void handleButtonROIs(bool checked);

private:
    void updateImage(const QImage image);
    void updateImageScaling(void);

    QImage current_image;

    QGraphicsScene* scene;
    QGraphicsView* view;
};

#endif // TURAG_CONSOLE_CONSOLE_FRONTEND_TINACAMERAFRONTEND_H
