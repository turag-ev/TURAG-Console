#ifndef FELDBUSBOOTLOADERVIEW_H
#define FELDBUSBOOTLOADERVIEW_H

#include <QWidget>
#include <tina++/feldbus/host/device.h>

class FeldbusBootloaderView : public QWidget
{
    Q_OBJECT
public:
    explicit FeldbusBootloaderView(TURAG::Feldbus::Device* dev, QWidget *parent = 0);

signals:

public slots:

private:
    TURAG::Feldbus::Device* dev;

};

#endif // FELDBUSBOOTLOADERVIEW_H
