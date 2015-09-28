#ifndef DEBUGPRINTCLASS_H
#define DEBUGPRINTCLASS_H

#include <QObject>
#include <QString>

class DebugPrintClass : public QObject
{
    Q_OBJECT
public:
    explicit DebugPrintClass(QObject *parent = 0) : QObject(parent) {  }

signals:
    void debugMsg(QString msg);

public slots:
    void generateDebugMessage(QString msg) {
        emit debugMsg(msg + "\n");
    }
};


extern DebugPrintClass rs485Debug;

#endif // DEBUGPRINTCLASS_H
