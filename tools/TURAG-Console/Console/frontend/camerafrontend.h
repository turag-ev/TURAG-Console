#ifndef TINACAMERAVIEW_H
#define TINACAMERAVIEW_H

#include <QStackedWidget>
#include <QList>
#include "basefrontend.h"

class QAction;
class QListWidget;

class TinaCameraFrontend : public BaseFrontend
{
    Q_OBJECT
public:
    explicit TinaCameraFrontend(QWidget *parent = 0);

signals:
    void newGraph(int index);

public slots:
    void writeLine(QByteArray line);
    virtual void writeData(QByteArray data);
    void activateGraph(int index);
    // frontend was connected to stream
    void onConnected(bool readOnly, bool isBuffered, QIODevice*);

    // frontend was disconnected from stream
    void onDisconnected(bool reconnecting);
    void clear(void);
    bool saveOutput(QString file);

private slots:
    void activateGraphInternal(int index);

private:
    QList<int> graphIndices;
    QStackedWidget* stack;
    QListWidget* graphlist;
};

#endif // TINACAMERAVIEW_H
