#include "feldbusaktorview.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <frontend/util/datagraph.h>
#include <QRadioButton>
#include <QCheckBox>
#include <QFormLayout>
#include <QSettings>
#include <QIntValidator>
#include <QTimer>
#include <QSignalMapper>
#include <QPalette>
#include <QColor>


FeldbusAktorView::FeldbusAktorView(Aktor *aktor, QWidget *parent) :
    QWidget(parent), actor(aktor), commandset(nullptr)
{
    intervalValidator = new QIntValidator(1, 100000, this);
    lengthValidator = new QIntValidator(1, 100000, this);

    QHBoxLayout* vlayout = new QHBoxLayout;
    QVBoxLayout* left_layout = new QVBoxLayout;
    QVBoxLayout* right_layout = new QVBoxLayout;
    value_grid = new QGridLayout;
    QVBoxLayout* settings_layout = new QVBoxLayout;
    QHBoxLayout* button_layout = new QHBoxLayout;

    vlayout->addLayout(left_layout);
    vlayout->addLayout(right_layout);
    vlayout->setStretch(1,100);
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
    updateDeviceValues->setDisabled(true);
    button_layout->addWidget(updateDeviceValues);
    connect(updateDeviceValues, SIGNAL(clicked()), this, SLOT(onUpdateDeviceValues()));

    QHBoxLayout* startStopLayout = new QHBoxLayout;
    settings_layout->addLayout(startStopLayout);
    QLabel* startDescr = new QLabel("zyklisch: Daten werden ständig vom Gerät geholt und angezeigt; einmalig: die Änderung eines Wertes startet die Datenaufzeichnung für die angegebene Dauer");
    startDescr->setWordWrap(true);
    startStopLayout->addWidget(startDescr);
    QVBoxLayout* startStopLayoutv = new QVBoxLayout;
    startStopDataUpdate = new QPushButton("Start");
    startStopDataUpdate->setDisabled(true);
    startStopLayoutv->addWidget(startStopDataUpdate);
    connect(startStopDataUpdate, SIGNAL(clicked()), this, SLOT(onStartStopDataUpdate()));
    updateDuration = new QLabel("");
    startStopLayoutv->addWidget(updateDuration);
    startStopLayout->addLayout(startStopLayoutv);

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
    QLabel* label2 = new QLabel("Aufzeichnungsdauer [Samples]");
    updateInterval = new QLineEdit;
    updateInterval->setValidator(intervalValidator);
    connect(updateInterval, SIGNAL(textEdited(QString)), this, SLOT(onInputEdited()));
    updateLength = new QLineEdit;
    updateLength->setValidator(lengthValidator);
    connect(updateLength, SIGNAL(textEdited(QString)), this, SLOT(onInputEdited()));
    settings_form->addRow(label1, updateInterval);
    settings_form->addRow(label2, updateLength);

    updateTimer = new QTimer(this);

    setMapper = new QSignalMapper(this);
    connect(setMapper, SIGNAL(mapped(int)), this, SLOT(onValueSet(int)));

    readSettings();
    onInputEdited();

}


void FeldbusAktorView::onGetCommandSet(void) {
    if (commandset) {
        if (updateTimer->isActive()) {
            onStartStopDataUpdate();
        }

        delete[] commandset;
        plot->clear();

        for (CommandsetEntry& entry : commandsetGrid) {
            entry.button->disconnect();
            entry.button->deleteLater();
            entry.checkbox->deleteLater();
            entry.label->deleteLater();
            entry.value->deleteLater();
        }
        commandsetGrid.clear();
    }

    unsigned commandsetLength = actor->getCommandsetLength();
    if (commandsetLength == 0) {
        return;
    }

    commandset = new AktorCommand_t[commandsetLength];
    if (!actor->populateCommandSet(commandset, commandsetLength)) {
        return;
    }

    char command_name[256];

    for (unsigned i = 0; i < commandsetLength; ++i) {
        if (commandset[i].length != AktorCommandLength::none) {
            CommandsetEntry entry;
            entry.key = i+1;
            entry.label = new QLabel;

            if (actor->getCommandName(entry.key, command_name)) {
                entry.label->setText(QString(command_name));
            } else {
                entry.label->setText("???");
            }
            entry.value = new QLineEdit;
            if (commandset[i].writeAccess == AktorCommandWriteAccess::no_write) {
                entry.value->setReadOnly(true);
                QPalette pal = entry.value->palette();
                QColor clr = this->palette().color(QPalette::Window);
                pal.setColor(QPalette::Active, QPalette::Base, clr);
                pal.setColor(QPalette::Inactive, QPalette::Base, clr);
                entry.value->setPalette(pal);
            }
            entry.button = new QPushButton("Set");
            connect(entry.button, SIGNAL(clicked()), setMapper, SLOT(map()));
            setMapper->setMapping(entry.button, commandsetGrid.size());

            entry.checkbox = new QCheckBox;

            value_grid->addWidget(entry.label, i, 0);
            value_grid->addWidget(entry.value, i, 1);
            value_grid->addWidget(entry.button, i, 2);
            value_grid->addWidget(entry.checkbox, i, 3);

            commandsetGrid.append(entry);
        }
    }
    updateDeviceValues->setDisabled(false);
    onUpdateDeviceValues();

}

void FeldbusAktorView::onUpdateDeviceValues(void) {
    for (CommandsetEntry& entry : commandsetGrid) {
        if (commandset[entry.key-1].factor == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE) {
            int32_t value;
            if (!actor->getValue(entry.key, &value)) {
                entry.value->setText("ERROR");
            } else {
                entry.value->setText(QString("%1").arg(value));
            }
        } else {
            float value;
            if (!actor->getValue(entry.key, &value)) {
                entry.value->setText("ERROR");
            } else {
                entry.value->setText(QString("%1").arg(value));
            }
        }
    }
}

void FeldbusAktorView::onCyclicDataUpdate(bool checked) {

}

void FeldbusAktorView::onOneShotDataUpdate(bool checked) {

}

void FeldbusAktorView::onStartStopDataUpdate(void) {
    validateInput();

    if (!updateTimer->isActive()) {
        startStopDataUpdate->setText("Stop");
        updateTimer->setInterval(updateInterval->text().toUInt());
        updateCounter = 0;
        plot->clear();

        if (oneShotDataUpdate->isChecked()) {

        } else {

        }
        updateTimer->start();
        onTimeout();
    } else {
        startStopDataUpdate->setText("Start");
        updateTimer->stop();
    }

}

void FeldbusAktorView::readSettings() {
    QSettings settings;
    settings.beginGroup("FeldbusAktorView");

    updateInterval->setText(settings.value("updateInterval", "10").toString());
    updateLength->setText(settings.value("updateLength", "100").toString());

    if (settings.value("cyclicDataUpdate", true).toBool()) {
        cyclicDataUpdate->setChecked(true);
    } else {
        oneShotDataUpdate->setChecked(true);
    }
}

void FeldbusAktorView::writeSettings() {
    validateInput();

    QSettings settings;
    settings.beginGroup("FeldbusAktorView");

    settings.setValue("updateInterval", updateInterval->text());
    settings.setValue("updateLength", updateLength->text());
    settings.setValue("cyclicDataUpdate", cyclicDataUpdate->isChecked());
}

FeldbusAktorView::~FeldbusAktorView(void) {
    if (commandset) {
        if (updateTimer->isActive()) {
            onStartStopDataUpdate();
        }
        delete[] commandset;
    }
    writeSettings();
}


void FeldbusAktorView::validateInput(void) {
    if (!updateInterval->hasAcceptableInput()) {
        updateInterval->setText("10");
    }
    if (!updateLength->hasAcceptableInput()) {
        updateLength->setText("100");
    }
}


void FeldbusAktorView::onInputEdited(void) {
    if (updateInterval->hasAcceptableInput() && updateLength->hasAcceptableInput()) {
        updateDuration->setText(QString("Aufzeichnungsfenster: %1 s").arg(updateInterval->text().toFloat() * updateLength->text().toFloat() / 1000.0f));
    } else {
        updateDuration->setText("");
    }

}

void FeldbusAktorView::onTimeout(void) {
    if (oneShotDataUpdate->isChecked()) {
        ++updateCounter;

        if (updateCounter > updateLength->text().toUInt()) {
            onStartStopDataUpdate();
        }
    }



}


void FeldbusAktorView::onValueSet(int id) {
    uint8_t key = commandsetGrid.at(id).key;
    if (commandset[key - 1].factor == TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE) {
        actor->setValue(key, static_cast<int32_t>(commandsetGrid.at(id).value->text().toInt()));
    } else {
        actor->setValue(key, commandsetGrid.at(id).value->text().toFloat());
    }

    if (oneShotDataUpdate->isChecked() && !updateTimer->isActive()) {
        onStartStopDataUpdate();
    }
}
