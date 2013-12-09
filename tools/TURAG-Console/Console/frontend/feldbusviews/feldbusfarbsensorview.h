#ifndef FELDBUSFARBSENSORVIEW_H
#define FELDBUSFARBSENSORVIEW_H

#include <QWidget>
#include <tina++/feldbus/host/farbsensor.h>

using namespace TURAG::Feldbus;

class FeldbusFarbsensorView : public QWidget
{
    Q_OBJECT
protected:
    Farbsensor* farbsensor_;

public:
    explicit FeldbusFarbsensorView(Farbsensor* farbsensor, QWidget *parent = 0);

signals:

public slots:

};

#endif // FELDBUSFARBSENSORVIEW_H
