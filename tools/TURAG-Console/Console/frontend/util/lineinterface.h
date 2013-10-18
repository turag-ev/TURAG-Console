#ifndef LINEINTERFACE_H
#define LINEINTERFACE_H

#include <QObject>
#include <QByteArray>

class LineInterface : public QObject
{
    Q_OBJECT

protected:
    void lineComplete(QByteArray line);
    QByteArray buffer_;

public:
    explicit LineInterface(QObject *parent = 0);
    
signals:
    void beginUpdate(void);
    void lineReady(QByteArray line);
    void endUpdate(void);
    
public slots:
    virtual void writeData(QByteArray data);
    
};

#endif // LINEINTERFACE_H
