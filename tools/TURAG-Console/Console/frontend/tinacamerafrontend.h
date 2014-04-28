#ifndef TINACAMERAVIEW_H
#define TINACAMERAVIEW_H

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
    void onConnected(bool readOnly, bool isBuffered, QIODevice*);

    // frontend was disconnected from stream
    void onDisconnected(bool reconnecting);
    void clear(void);
    bool saveOutput(QString file);

    // ui handler
    void handleButtonDump(void);

private:
    void updateImage(const QImage image);
    void updateImageScaling(void);

    QImage current_image;

    QGraphicsScene* scene;
    QGraphicsView* view;
};

#endif // TINACAMERAVIEW_H
