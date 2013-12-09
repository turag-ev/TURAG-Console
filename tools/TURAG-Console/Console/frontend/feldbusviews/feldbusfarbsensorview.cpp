#include "feldbusfarbsensorview.h"
#include <QVBoxLayout>
#include <QLabel>

FeldbusFarbsensorView::FeldbusFarbsensorView(Farbsensor *farbsensor, QWidget *parent) :
    QWidget(parent), farbsensor_(farbsensor)
{
    QHBoxLayout* hlayout = new QHBoxLayout;
    QLabel* title = new QLabel("Farbsensor-Analyse");
    hlayout->addWidget(title);
    setLayout(hlayout);
}
