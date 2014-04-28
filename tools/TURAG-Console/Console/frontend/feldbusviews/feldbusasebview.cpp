#include "feldbusasebview.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFont>
#include <QPalette>
#include <QColor>

FeldbusAsebView::FeldbusAsebView(Aseb* aseb, QWidget *parent) :
    QWidget(parent), aseb_(aseb), deviceWasRead_(false)
{
    QVBoxLayout* outer_layout = new QVBoxLayout;
    QGridLayout* info_layout = new QGridLayout;
    info_layout->setAlignment(Qt::AlignBottom);
    QHBoxLayout* actual_layout = new QHBoxLayout;

    QVBoxLayout* left_layout = new QVBoxLayout;
    QVBoxLayout* right_layout = new QVBoxLayout;
    digital_in_layout_ = new QGridLayout;
    analog_in_layout_ = new QGridLayout;
    digital_out_layout_ = new QGridLayout;
    pwm_out_layout_ = new QGridLayout;
    QVBoxLayout* button_layout = new QVBoxLayout;

    QLabel* label = new QLabel("Sync size:");
    info_layout->addWidget(label, 0, 0);
    syncBufferSize_ = new QLabel;
    info_layout->addWidget(syncBufferSize_, 0, 1);

    label = new QLabel("Analog input size:");
    info_layout->addWidget(label, 1, 0);
    analogInputSize_ = new QLabel;
    info_layout->addWidget(analogInputSize_, 1, 1);

    label = new QLabel("PWM output size:");
    info_layout->addWidget(label, 2, 0);
    pwmOutputSize_ = new QLabel;
    info_layout->addWidget(pwmOutputSize_, 2, 1);


    actual_layout->addLayout(left_layout);
    actual_layout->addLayout(right_layout);
    outer_layout->addLayout(actual_layout);
    outer_layout->addLayout(info_layout);
    setLayout(outer_layout);

    QLabel* input_label = new QLabel("Eingänge");
    left_layout->addWidget(input_label);
    left_layout->addLayout(digital_in_layout_);
    left_layout->addLayout(analog_in_layout_);
    left_layout->setAlignment(input_label, Qt::AlignTop);

    readDevice_ = new QPushButton("Gerät einlesen");
    connect(readDevice_, SIGNAL(clicked()), this, SLOT(onReadDevice()));
    left_layout->addWidget(readDevice_);
    left_layout->setAlignment(readDevice_, Qt::AlignBottom);

    QLabel* output_label = new QLabel("Ausgänge");
    right_layout->addWidget(output_label);
    right_layout->addLayout(digital_out_layout_);
    right_layout->addLayout(pwm_out_layout_);
    right_layout->addLayout(button_layout);
    right_layout->setAlignment(output_label, Qt::AlignTop);
    right_layout->setAlignment(button_layout, Qt::AlignBottom);

    resetOutputs_ = new QPushButton("Zurücksetzen");
    resetOutputs_->setDisabled(true);
    connect(resetOutputs_, SIGNAL(clicked()), this, SLOT(onResetOutputs()));
    button_layout->addWidget(resetOutputs_);

    setOutputs_ = new QPushButton("Werte setzen");
    setOutputs_->setDisabled(true);
    connect(setOutputs_, SIGNAL(clicked()), this, SLOT(onSetOutputs()));
    button_layout->addWidget(setOutputs_);

    QFont label_font = input_label->font();
    label_font.setPointSize(14);
    label_font.setWeight(QFont::Bold);
    input_label->setFont(label_font);
    output_label->setFont(label_font);

    connect(&updateTimer_, SIGNAL(timeout()), this, SLOT(onUpdate()));
}


void FeldbusAsebView::onReadDevice(void) {
    readDevice_->setEnabled(false);

    if (deviceWasRead_) {
        for (LabelCheckboxCombo& combo : digitalInputs_) {
            combo.checkbox->deleteLater();
            combo.label->deleteLater();
        }
        digitalInputs_.clear();

        for (LabelCheckboxCombo& combo : digitalOutputs_) {
            combo.checkbox->deleteLater();
            combo.label->deleteLater();
        }
        digitalOutputs_.clear();

        for (LabelLineeditCombo& combo : analogInputs_) {
            combo.label->deleteLater();
            combo.lineedit->deleteLater();
        }
        analogInputs_.clear();

        for (LabelLineeditCombo& combo : pwmOutputs_) {
            combo.label->deleteLater();
            combo.lineedit->deleteLater();
        }
        pwmOutputs_.clear();

        setOutputs_->setDisabled(true);
        resetOutputs_->setDisabled(true);

        updateTimer_.stop();

        delete[] asebAnalogInputSet_;
        delete[] asebPwmOutputSet_;
        delete[] asebSyncBuffer_;
    }

    int digInSize = 0;
    int digOutSize = 0;
    int analogInSize = 0;
    int pwmOutSize = 0;
    int syncSize = 0;
    unsigned analogResolution = 0;


    if (!aseb_->getDigitalInputSize(&digInSize) ||
            !aseb_->getDigitalOutputSize(&digOutSize) ||
            !aseb_->getPwmOutputSize(&pwmOutSize) ||
            !aseb_->getAnalogInputSize(&analogInSize) ||
            !aseb_->getSyncSize(&syncSize) ||
            !aseb_->getAnalogResolution(&analogResolution)) {
        readDevice_->setEnabled(true);
        return;
    }

    syncBufferSize_->setText(QString("%1").arg(syncSize));
    analogInputSize_->setText(QString("%1").arg(analogInSize));
    pwmOutputSize_->setText(QString("%1").arg(pwmOutSize));

    asebAnalogInputSet_ = new Aseb::Analog_t[analogInSize];
    asebPwmOutputSet_ = new Aseb::Pwm_t[pwmOutSize];
    asebSyncBuffer_ = new uint8_t[syncSize];

    if (aseb_->initialize(asebSyncBuffer_, syncSize, asebAnalogInputSet_, analogInSize, asebPwmOutputSet_, pwmOutSize)) {
        QLabel* label;
        QCheckBox* checkbox;
        QLineEdit* lineedit;
        char name[256];

        for (int i = 0; i < digInSize; ++i) {
            name[0] = 0;
            aseb_->getCommandName(i + TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_INPUT, name);
            label = new QLabel(QString(name));
            checkbox = new QCheckBox;
            checkbox->setEnabled(false);

            digitalInputs_.push_back(LabelCheckboxCombo(label, checkbox));
            digital_in_layout_->addWidget(label, i, 0);
            digital_in_layout_->addWidget(checkbox, i, 1);
        }

        for (int i = 0; i < digOutSize; ++i) {
            name[0] = 0;
            aseb_->getCommandName(i + TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT, name);
            label = new QLabel(QString(name));
            checkbox = new QCheckBox;

            digitalOutputs_.push_back(LabelCheckboxCombo(label, checkbox));
            digital_out_layout_->addWidget(label, i, 0);
            digital_out_layout_->addWidget(checkbox, i, 1);
        }

        for (int i = 0; i < analogInSize; ++i) {
            name[0] = 0;
            aseb_->getCommandName(i + TURAG_FELDBUS_ASEB_INDEX_START_ANALOG_INPUT, name);
            label = new QLabel(QString(name));

            unsigned resolution = 0;
            if (aseb_->getAnalogResolution(&resolution)) {
                label->setText(label->text() + QString("\n%1 Bit").arg(resolution));
            } else {
                label->setText("ERROR");
            }

            lineedit = new QLineEdit;
            QPalette pal = lineedit->palette();
            QColor clr = this->palette().color(QPalette::Window);
            pal.setColor(QPalette::Active, QPalette::Base, clr);
            pal.setColor(QPalette::Inactive, QPalette::Base, clr);
            lineedit->setPalette(pal);
            lineedit->setDisabled(true);

            analogInputs_.push_back(LabelLineeditCombo(label, lineedit));
            analog_in_layout_->addWidget(label, i, 0);
            analog_in_layout_->addWidget(lineedit, i, 1);
        }

        for (int i = 0; i < pwmOutSize; ++i) {
            name[0] = 0;
            aseb_->getCommandName(i + TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT, name);
            label = new QLabel(QString(name));

            unsigned frequency;
            if (aseb_->getPwmFrequency(i + TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT, &frequency)) {
                label->setText(label->text() + QString("\n%1 Hz").arg(frequency));
            } else {
                label->setText("ERROR");
            }

            lineedit = new QLineEdit;

            pwmOutputs_.push_back(LabelLineeditCombo(label, lineedit));
            pwm_out_layout_->addWidget(label, i, 0);
            pwm_out_layout_->addWidget(lineedit, i, 1);
        }

        updateTimer_.start(25);
        deviceWasRead_ = true;
        setOutputs_->setDisabled(false);
        resetOutputs_->setDisabled(false);

        onResetOutputs();
    } else {
        delete[] asebAnalogInputSet_;
        delete[] asebPwmOutputSet_;
        delete[] asebSyncBuffer_;
    }

    readDevice_->setEnabled(true);
}

void FeldbusAsebView::onResetOutputs(void) {
    unsigned key = TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT;
    for (LabelCheckboxCombo& combo : digitalOutputs_) {
        combo.checkbox->setChecked(aseb_->getDigitalOutput(key));
        ++key;
    }
    key = TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;
    for (LabelLineeditCombo& combo : pwmOutputs_) {
        combo.lineedit->setText(QString("%1").arg(aseb_->getPwmOutput(key)));
        ++key;
    }
}

void FeldbusAsebView::onSetOutputs(void) {
    unsigned key = TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT;
    for (LabelCheckboxCombo& combo : digitalOutputs_) {
        aseb_->setDigitalOutput(key, combo.checkbox->isChecked());
        ++key;
    }
    key = TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;
    for (LabelLineeditCombo& combo : pwmOutputs_) {
        aseb_->setPwmOutput(key, combo.lineedit->text().toFloat());
        ++key;
    }
}

void FeldbusAsebView::onUpdate(void) {
    if (aseb_->sync()) {
        unsigned key = TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_INPUT;
        for (LabelCheckboxCombo& combo : digitalInputs_) {
            combo.checkbox->setChecked(aseb_->getDigitalInput(key));
            ++key;
        }

        key = TURAG_FELDBUS_ASEB_INDEX_START_ANALOG_INPUT;
        for (LabelLineeditCombo& combo : analogInputs_) {
            combo.lineedit->setText(QString("%1").arg(aseb_->getAnalogInput(key)));
            ++key;
        }
    } else {
        for (LabelCheckboxCombo& combo : digitalInputs_) {
            combo.checkbox->setChecked(false);
        }
        for (LabelLineeditCombo& combo : analogInputs_) {
            combo.lineedit->setText("ERROR");
        }
    }
}
