#include "feldbusaktorview.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <frontend/util/datagraph.h>
#include <QRadioButton>
#include <QCheckBox>
#include <QFormLayout>

FeldbusAktorView::FeldbusAktorView(Aktor *aktor, QWidget *parent) :
    QWidget(parent), actor(aktor)
{
    QHBoxLayout* vlayout = new QHBoxLayout;
    QVBoxLayout* left_layout = new QVBoxLayout;
    QVBoxLayout* right_layout = new QVBoxLayout;
    QGridLayout* value_grid = new QGridLayout;
    QVBoxLayout* settings_layout = new QVBoxLayout;
    QHBoxLayout* button_layout = new QHBoxLayout;

    vlayout->addLayout(left_layout);
    vlayout->addLayout(right_layout);
    setLayout(vlayout);

    left_layout->addLayout(value_grid);
    QVBoxLayout* left_sub_layout = new QVBoxLayout;
    left_layout->addLayout(left_sub_layout);
    left_sub_layout->addLayout(button_layout);
    QLabel* descr = new QLabel("Markierte Werte werden in der Diagrammanzeige berücksichtigt");
    descr->setWordWrap(true);
    left_sub_layout->addWidget(descr);
    left_layout->setAlignment(left_sub_layout, Qt::AlignBottom);


    plot = new DataGraph;
    right_layout->addWidget(plot);
    right_layout->addLayout(settings_layout);

    getCommandSet = new QPushButton("Commandset\nermitteln");
    button_layout->addWidget(getCommandSet);
    connect(getCommandSet, SIGNAL(clicked()), this, SLOT(onGetCommandSet()));

    updateDeviceValues = new QPushButton("Werte\naktualisieren");
    button_layout->addWidget(updateDeviceValues);
    connect(updateDeviceValues, SIGNAL(clicked()), this, SLOT(onUpdateDeviceValues()));

    QHBoxLayout* startStopLayout = new QHBoxLayout;
    settings_layout->addLayout(startStopLayout);
    QLabel* startDescr = new QLabel("zyklisch: Daten werden ständig vom Gerät geholt und angezeigt; einmalig: die Änderung eines Wertes startet die Datenaufzeichnung für die angegebene Dauer");
    startDescr->setWordWrap(true);
    startStopLayout->addWidget(startDescr);
    startStopDataUpdate = new QPushButton("Start");
    startStopLayout->addWidget(startStopDataUpdate);

    QHBoxLayout* radio_layout = new QHBoxLayout;
    settings_layout->addLayout(radio_layout);
    cyclicDataUpdate = new QRadioButton("zyklische Datenaktualisierung");
    radio_layout->addWidget(cyclicDataUpdate);
    connect(cyclicDataUpdate, SIGNAL(toggled(bool)), this, SLOT(onCyclicDataUpdate(bool)));
    oneShotDataUpdate = new QRadioButton("einmalige Datenaktualisierung");
    radio_layout->addWidget(oneShotDataUpdate);
    connect(oneShotDataUpdate, SIGNAL(toggled(bool)), this, SLOT(onOneShotDataUpdate(bool)));

    QFormLayout* settings_form = new QFormLayout;
    settings_layout->addLayout(settings_form);
    QLabel* label1 = new QLabel("Abfrage-Intervall [ms]");
    QLabel* label2 = new QLabel("Aufzeichnungsdauer [ms]");
    updateInterval = new QLineEdit;
    updateLength = new QLineEdit;
    settings_form->addRow(label1, updateInterval);
    settings_form->addRow(label2, updateLength);



}


void FeldbusAktorView::onGetCommandSet(void) {

}

void FeldbusAktorView::onUpdateDeviceValues(void) {

}

void FeldbusAktorView::onCyclicDataUpdate(bool checked) {

}

void FeldbusAktorView::onOneShotDataUpdate(bool checked) {

}
