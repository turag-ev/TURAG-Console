#include "feldbusasebview.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFont>
#include <QPalette>
#include <QColor>
#include <QScrollArea>

FeldbusAsebView::FeldbusAsebView(Aseb* aseb, QWidget *parent) :
    QWidget(parent), aseb_(aseb), asebAnalogInputSet_(nullptr),
    asebPwmOutputSet_(nullptr), asebSyncBuffer_(nullptr)
{
    QHBoxLayout* actual_layout = new QHBoxLayout;
    QGridLayout* info_layout = new QGridLayout;
    info_layout->setAlignment(Qt::AlignBottom);

    QVBoxLayout* left_layout = new QVBoxLayout;
    QVBoxLayout* right_layout = new QVBoxLayout;
    digital_in_layout_ = new QGridLayout;
    analog_in_layout_ = new QGridLayout;
    digital_out_layout_ = new QGridLayout;
    pwm_out_layout_ = new QGridLayout;
    QHBoxLayout* button_layout = new QHBoxLayout;

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

    QScrollArea* scrollarea = new QScrollArea;
    QFrame* scrollchild = new QFrame;
    scrollchild->setLayout(actual_layout);
    actual_layout->addLayout(left_layout);
    actual_layout->addLayout(right_layout);
    actual_layout->setAlignment(left_layout, Qt::AlignTop);
    actual_layout->setAlignment(right_layout, Qt::AlignTop);

    scrollarea->setWidgetResizable(true);
    scrollarea->setWidget(scrollchild);
    QLabel* input_label = new QLabel("Eingänge");
    left_layout->addWidget(input_label);
    left_layout->addLayout(digital_in_layout_);
    left_layout->addLayout(analog_in_layout_);
    left_layout->setAlignment(input_label, Qt::AlignTop);

    QLabel* output_label = new QLabel("Ausgänge");
    right_layout->addWidget(output_label);
    right_layout->addLayout(digital_out_layout_);
    right_layout->addLayout(pwm_out_layout_);

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

    QVBoxLayout* outer_layout = new QVBoxLayout;
    outer_layout->addWidget(scrollarea);
    outer_layout->addLayout(button_layout);
    outer_layout->addLayout(info_layout);
    setLayout(outer_layout);

    connect(&updateTimer_, SIGNAL(timeout()), this, SLOT(onUpdate()));



    /*
     * read device
     */
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

            connect(checkbox, SIGNAL(toggled(bool)), this, SLOT(onUserInput()));
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
            connect(lineedit, SIGNAL(textEdited(QString)), this, SLOT(onUserInput()));
            connect(lineedit, SIGNAL(returnPressed()), this, SLOT(onSetOutputs()));

            pwmOutputs_.push_back(LabelLineeditCombo(label, lineedit));
            pwm_out_layout_->addWidget(label, i, 0);
            pwm_out_layout_->addWidget(lineedit, i, 1);
        }

        updateTimer_.start(25);
        setOutputs_->setDisabled(false);
        resetOutputs_->setDisabled(false);

        onResetOutputs();
    }
}


FeldbusAsebView::~FeldbusAsebView(void) {
    for (LabelCheckboxCombo& combo : digitalInputs_) {
        combo.checkbox->deleteLater();
        combo.label->deleteLater();
    }

    for (LabelCheckboxCombo& combo : digitalOutputs_) {
        combo.checkbox->deleteLater();
        combo.label->deleteLater();
    }

    for (LabelLineeditCombo& combo : analogInputs_) {
        combo.label->deleteLater();
        combo.lineedit->deleteLater();
    }

    for (LabelLineeditCombo& combo : pwmOutputs_) {
        combo.label->deleteLater();
        combo.lineedit->deleteLater();
    }

    if (asebAnalogInputSet_) delete[] asebAnalogInputSet_;
    if (asebPwmOutputSet_)    delete[] asebPwmOutputSet_;
    if (asebSyncBuffer_)   delete[] asebSyncBuffer_;
}


void FeldbusAsebView::onResetOutputs(void) {
    unsigned key = TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT;
    for (LabelCheckboxCombo& combo : digitalOutputs_) {
        combo.checkbox->setChecked(aseb_->getDigitalOutput(key));
        QPalette pal = combo.checkbox->palette();
        pal.setColor(QPalette::Active, QPalette::Base, Qt::white);
        combo.checkbox->setPalette(pal);
        ++key;
    }
    key = TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;
    for (LabelLineeditCombo& combo : pwmOutputs_) {
        combo.lineedit->setText(QString("%1").arg(aseb_->getPwmOutput(key)));
        QPalette pal = combo.lineedit->palette();
        pal.setColor(QPalette::Active, QPalette::Base, Qt::white);
        combo.lineedit->setPalette(pal);
        ++key;
    }
}

void FeldbusAsebView::onSetOutputs(void) {
    unsigned key = TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT;
    for (LabelCheckboxCombo& combo : digitalOutputs_) {
        aseb_->setDigitalOutput(key, combo.checkbox->isChecked());
        QPalette pal = combo.checkbox->palette();
        pal.setColor(QPalette::Active, QPalette::Base, Qt::white);
        combo.checkbox->setPalette(pal);
        ++key;
    }
    key = TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT;
    for (LabelLineeditCombo& combo : pwmOutputs_) {
        aseb_->setPwmOutput(key, combo.lineedit->text().toFloat());
        QPalette pal = combo.lineedit->palette();
        pal.setColor(QPalette::Active, QPalette::Base, Qt::white);
        combo.lineedit->setPalette(pal);
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

void FeldbusAsebView::onUserInput(void) {
    QWidget* widget = static_cast<QWidget*>(sender());

    QPalette pal = widget->palette();
    pal.setColor(QPalette::Active, QPalette::Base, Qt::red);
    widget->setPalette(pal);
}
