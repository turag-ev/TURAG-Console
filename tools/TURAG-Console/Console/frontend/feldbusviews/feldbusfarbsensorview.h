#ifndef FELDBUSFARBSENSORVIEW_H
#define FELDBUSFARBSENSORVIEW_H

#include <QWidget>
#include <tina++/feldbus/host/farbsensor.h>

class DataGraph;
class QTimer;
class QTime;
class QGraphicsScene;

using namespace TURAG::Feldbus;

class FeldbusFarbsensorView : public QWidget
{
    Q_OBJECT
protected:
    Farbsensor* farbsensor_;
    DataGraph* rgbGraph;
    DataGraph* hsvGraph;
    QTimer* mainInterval;
    QTime* time_;

    QGraphicsScene* rgbScene;
    QGraphicsScene* hsvScene;

protected slots:
    void mainIntervalTimeout();

public:
    explicit FeldbusFarbsensorView(Farbsensor* farbsensor, QWidget *parent = 0);
    ~FeldbusFarbsensorView();

signals:

public slots:

};

#endif // FELDBUSFARBSENSORVIEW_H
