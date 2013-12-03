#ifndef TINAINTERFACE_H
#define TINAINTERFACE_H

#include <QObject>

class TinaInterface : public QObject
{
    Q_OBJECT

protected:
    QByteArray packageBuffer_;
    QByteArray trimCarriageReturn(QByteArray data);

public:
    explicit TinaInterface(QObject *parent = 0);
    
signals:
    void beginUpdate(void);
    void tinaPackageReady(QByteArray line);
    void cmenuDataReady(QByteArray data);
    void endUpdate(void);

public slots:
    virtual void dataInput(QByteArray data);

};

#endif // TINAINTERFACE_H
