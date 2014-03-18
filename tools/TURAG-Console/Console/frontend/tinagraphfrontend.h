#ifndef TINAGRAPHVIEW_H
#define TINAGRAPHVIEW_H

#include <QStackedWidget>
#include <QList>
#include "basefrontend.h"

class QAction;
class QListWidget;

class TinaGraphFrontend : public BaseFrontend
{
    Q_OBJECT
public:
    explicit TinaGraphFrontend(QWidget *parent = 0);
    ~TinaGraphFrontend(void);

signals:
    void newGraph(int index);

public slots:
    void writeLine(QByteArray line);
    virtual void writeData(QByteArray data);
    void activateGraph(int index);
    // frontend was connected to stream
    void onConnected(bool readOnly, bool isSequential, QIODevice*);

    // frontend was disconnected from stream
    void onDisconnected(bool reconnecting);
    void clear(void);
    bool saveOutput(void);

private slots:
    void onZoomAuto(void);
    void onSetZoomer(void);
    void onDragGraph(void);
    void activateGraphInternal(int index);

private:
    QList<int> graphIndices;
    QStackedWidget* stack;
    QAction* zoom_box_zoom_action;
    QAction* zoom_drag_action;
    QListWidget* graphlist;
};

#endif // TINAGRAPHVIEW_H
