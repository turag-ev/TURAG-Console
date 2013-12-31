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
    QVBoxLayout* vlayout = new QVBoxLayout;
    QHBoxLayout* left_layout = new QHBoxLayout;
    QHBoxLayout* right_layout = new QHBoxLayout;
    QGridLayout* value_grid = new QGridLayout;
    QHBoxLayout* settings_layout = new QHBoxLayout;
    QHBoxLayout* button_layout = new QHBoxLayout;

    vlayout->addLayout(left_layout);
    vlayout->addLayout(right_layout);
    setLayout(vlayout);

    left_layout->addLayout(value_grid);
    left_layout->addLayout(button_layout);

    plot = new DataGraph;
    right_layout->addWidget(plot);
    right_layout->addLayout(settings_layout);

    getCommandSet = new QPushButton("Commandset ermitteln");
    button_layout->addWidget(getCommandSet);
    connect(getCommandSet, SIGNAL(clicked()), this, SLOT(onGetCommandSet()));

    updateDeviceValues = new QPushButton("Werte schreiben & aktualisieren");
    button_layout->addWidget(updateDeviceValues);
    connect(updateDeviceValues, SIGNAL(clicked()), this, SLOT(onUpdateDeviceValues()));

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
