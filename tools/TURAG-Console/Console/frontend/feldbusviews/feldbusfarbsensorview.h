#ifndef FELDBUSFARBSENSORVIEW_H
#define FELDBUSFARBSENSORVIEW_H

#include <QWidget>
#include <tina++/feldbus/host/farbsensor.h>
#include <frontend/graphutils/datagraph.h>

class DataGraph;
class QTimer;
class QTime;
class QGraphicsScene;

using namespace TURAG::Feldbus;

class FeldbusFarbsensorView : public QWidget
{
    Q_OBJECT
public:
    explicit FeldbusFarbsensorView(Farbsensor* farbsensor, QWidget *parent = 0);
    ~FeldbusFarbsensorView();
    QList<QAction *> getActions(void) { return rgbGraph->getActions(); }

protected slots:
    void mainIntervalTimeout();

protected:
    Farbsensor* farbsensor_;
    DataGraph* rgbGraph;
    DataGraph* hsvGraph;
    QTimer* mainInterval;
    QTime* time_;

    QGraphicsScene* rgbScene;
    QGraphicsScene* hsvScene;

signals:

public slots:

};

#endif // FELDBUSFARBSENSORVIEW_H
