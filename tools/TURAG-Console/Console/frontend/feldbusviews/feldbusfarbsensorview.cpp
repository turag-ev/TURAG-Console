#include "feldbusfarbsensorview.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <frontend/util/datagraph.h>
#include <QTimer>
#include <QTime>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGridLayout>

using namespace TURAG::Feldbus;


FeldbusFarbsensorView::FeldbusFarbsensorView(Farbsensor *farbsensor, QWidget *parent) :
    QWidget(parent), farbsensor_(farbsensor)
{
    QGridLayout* layout = new QGridLayout;
    QLabel* title = new QLabel("<br>Farbsensor-Analyse</br>");
    layout->addWidget(title, 0, 0, 1, 2);

    rgbGraph = new DataGraph;
    hsvGraph = new DataGraph;
    layout->addWidget(rgbGraph, 1, 0);
    layout->addWidget(hsvGraph, 1, 1);

    rgbScene = new QGraphicsScene(QRectF(0 ,0, 150, 150), this);
    QGraphicsView* rgbView = new QGraphicsView(rgbScene);
    layout->addWidget(rgbView, 2, 0);

    hsvScene = new QGraphicsScene(QRectF(0 ,0, 150, 150), this);
    QGraphicsView* hsvView = new QGraphicsView(hsvScene);
    layout->addWidget(hsvView, 2, 1);



    setLayout(layout);

    rgbGraph->addChannel("R", (qreal)5, (qreal)0, (qreal)256);
    rgbGraph->addChannel("G", (qreal)5, (qreal)0, (qreal)256);
    rgbGraph->addChannel("B", (qreal)5, (qreal)0, (qreal)256);

    hsvGraph->addChannel("H", (qreal)5, (qreal)0, (qreal)256);
    hsvGraph->addChannel("S", (qreal)5, (qreal)0, (qreal)256);
    hsvGraph->addChannel("V", (qreal)5, (qreal)0, (qreal)256);

    time_ = new QTime();
    time_->start();

    mainInterval = new QTimer(this);
    connect(mainInterval, SIGNAL(timeout()), this, SLOT(mainIntervalTimeout()));
    mainInterval->start(100);
}

FeldbusFarbsensorView::~FeldbusFarbsensorView() {
    delete time_;
}

void FeldbusFarbsensorView::mainIntervalTimeout() {
    Farbsensor::rgb_t rgb;
    if (farbsensor_->getRGB(&rgb)) {
        qreal currentTime = static_cast<qreal>(time_->elapsed()) / 1000;

        rgbGraph->addData(0, QPointF(currentTime, rgb.r));
        rgbGraph->addData(1, QPointF(currentTime, rgb.g));
        rgbGraph->addData(2, QPointF(currentTime, rgb.b));

        rgbScene->setBackgroundBrush(QBrush(QColor(rgb.r, rgb.g, rgb.b)));
    }

    Farbsensor::hsv_t hsv;
    if (farbsensor_->getHSV(&hsv)) {
        qreal currentTime = static_cast<qreal>(time_->elapsed()) / 1000;

        hsvGraph->addData(0, QPointF(currentTime, hsv.h));
        hsvGraph->addData(1, QPointF(currentTime, hsv.s));
        hsvGraph->addData(2, QPointF(currentTime, hsv.v));

        QColor color;
        color.setHsv(hsv.h / (6*42) * 360, hsv.s, hsv.v);
        hsvScene->setBackgroundBrush(QBrush(color));
    }
}

