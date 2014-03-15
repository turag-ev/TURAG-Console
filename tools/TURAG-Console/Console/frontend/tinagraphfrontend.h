#ifndef TINAGRAPHVIEW_H
#define TINAGRAPHVIEW_H

#include <QStackedWidget>
#include <QList>
#include "basefrontend.h"

class TinaGraphFrontend : public BaseFrontend
{
    Q_OBJECT
public:
    explicit TinaGraphFrontend(QWidget *parent = 0);

signals:

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


private:
    QList<int> graphIndices;
    QStackedWidget* stack;

};

#endif // TINAGRAPHVIEW_H
