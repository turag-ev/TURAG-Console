#include "feldbusasebview.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <libs/lineeditext.h>
#include <QFont>
#include <QPalette>
#include <QColor>
#include <QScrollArea>
#include <QIntValidator>
#include "frontend/graphutils/datagraph.h"

FeldbusAsebView::FeldbusAsebView(Aseb* aseb, QWidget *parent) :
    QWidget(parent), aseb_(aseb), asebAnalogInputSet_(nullptr),
    asebPwmOutputSet_(nullptr), asebSyncBuffer_(nullptr)
{
    // top detail area
    // -----------------------------------------
    QGridLayout* detail_layout = new QGridLayout;

    QFrame* scrollchild = new QFrame;
    scrollchild->setLayout(detail_layout);
    QScrollArea* scrollarea = new QScrollArea;
    scrollarea->setWidgetResizable(true);
    scrollarea->setWidget(scrollchild);

    QLabel* input_label = new QLabel("Eingänge");
    QLabel* output_label = new QLabel("Ausgänge");
    QFont label_font = input_label->font();
    label_font.setPointSize(14);
    label_font.setWeight(QFont::Bold);
    input_label->setFont(label_font);
    output_label->setFont(label_font);

    // button area
    // -----------------------------------------
    QHBoxLayout* button_layout = new QHBoxLayout;
    resetOutputs_ = new QPushButton("Zurücksetzen");
    resetOutputs_->setDisabled(true);
    connect(resetOutputs_, SIGNAL(clicked()), this, SLOT(onResetOutputs()));
    button_layout->addWidget(resetOutputs_);

    setOutputs_ = new QPushButton("Werte setzen");
    setOutputs_->setDisabled(true);
    connect(setOutputs_, SIGNAL(clicked()), this, SLOT(onSetOutputs()));
    button_layout->addWidget(setOutputs_);

    // info area at the bottom
    // -----------------------------------------
    QGridLayout* info_layout = new QGridLayout;
    info_layout->setAlignment(Qt::AlignBottom);

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

    // datagraph area on the right
    // -----------------------------------------
    QVBoxLayout* right_layout = new QVBoxLayout;
    QVBoxLayout* settings_layout = new QVBoxLayout;

    plot = new DataGraph;
    right_layout->addWidget(plot);
    right_layout->addLayout(settings_layout);

    updateDuration = new QLabel("");
    settings_layout->addWidget(updateDuration);

    QIntValidator* intervalValidator = new QIntValidator(1, 100000, this);
    QIntValidator* lengthValidator = new QIntValidator(1, 100000, this);

    QFormLayout* settings_form = new QFormLayout;
    settings_layout->addLayout(settings_form);
    QLabel* label1 = new QLabel("Abfrage-Intervall [ms]");
    QLabel* label2 = new QLabel("Aufzeichnungsdauer [Samples]");
    updateInterval = new LineEditExt("FeldbusAsebViewUpdateInterval", "10");
    updateInterval->setValidator(intervalValidator);
    connect(updateInterval, SIGNAL(textEdited(QString)), this, SLOT(onInputEdited()));
    updateLength = new LineEditExt("FeldbusAsebViewUpdateLength", "100");
    updateLength->setValidator(lengthValidator);
    connect(updateLength, SIGNAL(textEdited(QString)), this, SLOT(onInputEdited()));
    settings_form->addRow(label1, updateInterval);
    settings_form->addRow(label2, updateLength);


    // wrap it all together
    // -----------------------------------------
    QVBoxLayout* left_layout = new QVBoxLayout;
    left_layout->addWidget(scrollarea);
    left_layout->addLayout(button_layout);
    left_layout->addLayout(info_layout);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->addLayout(left_layout);
    layout->addLayout(right_layout);
    layout->setStretch(1, 100);
    setLayout(layout);


    if (!aseb_) return;


    connect(&updateTimer_, SIGNAL(timeout()), this, SLOT(onUpdate()));
    onInputEdited();


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
		QLabel* caption;
        QCheckBox* checkbox;
        QLineEdit* lineedit;
        QCheckBox* select_checkbox;
        char name[256];
        int row = 0;

        if (digInSize > 0 || analogInSize > 0 || digOutSize > 0 || pwmOutSize > 0) {
            detail_layout->addWidget(new QLabel("log"), row, 2);
        }

        if (digInSize > 0 || analogInSize > 0) {
            detail_layout->addWidget(input_label, row++, 0, 1, 2);
        }

        for (int i = 0; i < digInSize; ++i) {
            name[0] = 0;
            aseb_->getCommandName(i + TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_INPUT, name);
			caption = new QLabel(QString("%1: %2").arg(i).arg(name));
            checkbox = new QCheckBox;
            checkbox->setEnabled(false);
            select_checkbox = new QCheckBox;
            connect(select_checkbox, SIGNAL(toggled(bool)), this, SLOT(updateGraph()));

			digitalInputs_.push_back(LabelCheckboxCombo(caption, checkbox, select_checkbox));
			detail_layout->addWidget(caption, row, 0);
            detail_layout->addWidget(checkbox, row, 1);
            detail_layout->addWidget(select_checkbox, row, 2);
            ++row;
        }

        for (int i = 0; i < analogInSize; ++i) {
            name[0] = 0;
            aseb_->getCommandName(i + TURAG_FELDBUS_ASEB_INDEX_START_ANALOG_INPUT, name);
			caption = new QLabel(QString("%1: %2").arg(i).arg(name));

            unsigned resolution = 0;
            if (aseb_->getAnalogResolution(&resolution)) {
				caption->setText(caption->text() + QString("\n%1 Bit").arg(resolution));
            } else {
				caption->setText("ERROR");
            }

            lineedit = new QLineEdit;
            QPalette pal = lineedit->palette();
            QColor clr = this->palette().color(QPalette::Window);
            pal.setColor(QPalette::Active, QPalette::Base, clr);
            pal.setColor(QPalette::Inactive, QPalette::Base, clr);
            lineedit->setPalette(pal);
            lineedit->setDisabled(true);
            select_checkbox = new QCheckBox;
            connect(select_checkbox, SIGNAL(toggled(bool)), this, SLOT(updateGraph()));

			analogInputs_.push_back(LabelLineeditCombo(caption, lineedit, select_checkbox));
			detail_layout->addWidget(caption, row, 0);
            detail_layout->addWidget(lineedit, row, 1);
            detail_layout->addWidget(select_checkbox, row, 2);
            ++row;
        }

        if (digOutSize > 0 || pwmOutSize > 0) {
            if (digInSize > 0 || analogInSize > 0) {
                detail_layout->setRowMinimumHeight(row++, 10);
            }
            detail_layout->addWidget(output_label, row++, 0, 1, 2);
        }

        for (int i = 0; i < digOutSize; ++i) {
            name[0] = 0;
            aseb_->getCommandName(i + TURAG_FELDBUS_ASEB_INDEX_START_DIGITAL_OUTPUT, name);
			caption = new QLabel(QString("%1: %2").arg(i).arg(name));
			checkbox = new QCheckBox;
            select_checkbox = new QCheckBox;
            connect(select_checkbox, SIGNAL(toggled(bool)), this, SLOT(updateGraph()));

			digitalOutputs_.push_back(LabelCheckboxCombo(caption, checkbox, select_checkbox));
			detail_layout->addWidget(caption, row, 0);
            detail_layout->addWidget(checkbox, row, 1);
            detail_layout->addWidget(select_checkbox, row, 2);
            ++row;

            connect(checkbox, SIGNAL(toggled(bool)), this, SLOT(onUserInput()));
        }

        for (int i = 0; i < pwmOutSize; ++i) {
            name[0] = 0;
            aseb_->getCommandName(i + TURAG_FELDBUS_ASEB_INDEX_START_PWM_OUTPUT, name);
            //target
            caption = new QLabel(QString("%1: %2 target [%]").arg(i).arg(name));

            caption->setText(caption->text() + QString("\n%1 Hz").arg(aseb_->getPwmFrequency(i)));

            lineedit = new QLineEdit;
            connect(lineedit, SIGNAL(textEdited(QString)), this, SLOT(onUserInput()));
            connect(lineedit, SIGNAL(returnPressed()), this, SLOT(onSetOutputs()));
            select_checkbox = new QCheckBox;
            connect(select_checkbox, SIGNAL(toggled(bool)), this, SLOT(updateGraph()));
            pwmTargets_.push_back(LabelLineeditCombo(caption, lineedit, select_checkbox));

			detail_layout->addWidget(caption, row, 0);
            detail_layout->addWidget(lineedit, row, 1);
            detail_layout->addWidget(select_checkbox, row, 2);
            ++row;
            //value
            caption = new QLabel(QString("%1: %2 value [%]").arg(i).arg(name));

            lineedit = new QLineEdit;
            lineedit->setEnabled(false);
            select_checkbox = new QCheckBox;
            connect(select_checkbox, SIGNAL(toggled(bool)), this, SLOT(updateGraph()));

            pwmOutputs_.push_back(LabelLineeditCombo(caption, lineedit, select_checkbox));
            detail_layout->addWidget(caption, row, 0);
            detail_layout->addWidget(lineedit, row, 1);
            detail_layout->addWidget(select_checkbox, row, 2);
            ++row;

            //speed
            caption = new QLabel(QString("%1: %2 speed [%/s]").arg(i).arg(name));

            lineedit = new QLineEdit;
            connect(lineedit, SIGNAL(textEdited(QString)), this, SLOT(onUserInput()));
            connect(lineedit, SIGNAL(returnPressed()), this, SLOT(onSetOutputs()));
            select_checkbox = new QCheckBox;
            connect(select_checkbox, SIGNAL(toggled(bool)), this, SLOT(updateGraph()));

            pwmSpeeds_.push_back(LabelLineeditCombo(caption, lineedit, select_checkbox));
            detail_layout->addWidget(caption, row, 0);
            detail_layout->addWidget(lineedit, row, 1);
            detail_layout->addWidget(select_checkbox, row, 2);
            ++row;
        }

        detail_layout->setRowStretch(row, 2);

        setOutputs_->setDisabled(false);
        resetOutputs_->setDisabled(false);

        onResetOutputs();
    }
}


FeldbusAsebView::~FeldbusAsebView(void) {
    if (asebAnalogInputSet_) delete[] asebAnalogInputSet_;
    if (asebPwmOutputSet_)    delete[] asebPwmOutputSet_;
    if (asebSyncBuffer_)   delete[] asebSyncBuffer_;
}


void FeldbusAsebView::onResetOutputs(void) {
	unsigned key = 0;
    for (LabelCheckboxCombo& combo : digitalOutputs_) {
        combo.checkbox->setChecked(aseb_->getDigitalOutput(key));
        QPalette pal = combo.checkbox->palette();
        pal.setColor(QPalette::Active, QPalette::Base, Qt::white);
        combo.checkbox->setPalette(pal);
        ++key;
    }
    key = 0;
    for (LabelLineeditCombo& combo : pwmTargets_) {
        combo.lineedit->setText(QString("%1").arg(aseb_->getTargetPwmOutput(key)));
        QPalette pal = combo.lineedit->palette();
        pal.setColor(QPalette::Active, QPalette::Base, Qt::white);
        combo.lineedit->setPalette(pal);
        ++key;
    }
    key = 0;
    for (LabelLineeditCombo& combo : pwmSpeeds_) {
        combo.lineedit->setText(QString("%1").arg(aseb_->getPwmSpeed(key)));
        QPalette pal = combo.lineedit->palette();
        pal.setColor(QPalette::Active, QPalette::Base, Qt::white);
        combo.lineedit->setPalette(pal);
        ++key;
    }
}

void FeldbusAsebView::onSetOutputs(void) {
    unsigned key = 0;
    for (LabelCheckboxCombo& combo : digitalOutputs_) {
        aseb_->setDigitalOutput(key, combo.checkbox->isChecked());
        QPalette pal = combo.checkbox->palette();
        pal.setColor(QPalette::Active, QPalette::Base, Qt::white);
        combo.checkbox->setPalette(pal);
        ++key;
    }
    key = 0;
    for (LabelLineeditCombo& combo : pwmTargets_) {
        aseb_->setTargetPwmOutput(key, combo.lineedit->text().toFloat());
        QPalette pal = combo.lineedit->palette();
        pal.setColor(QPalette::Active, QPalette::Base, Qt::white);
        combo.lineedit->setPalette(pal);
        ++key;
    }
    key = 0;
    for (LabelLineeditCombo& combo : pwmSpeeds_) {
        aseb_->setPwmSpeed(key, combo.lineedit->text().toFloat());
        QPalette pal = combo.lineedit->palette();
        pal.setColor(QPalette::Active, QPalette::Base, Qt::white);
        combo.lineedit->setPalette(pal);
        ++key;
    }
	onResetOutputs();
}

void FeldbusAsebView::onUpdate(void) {
    int msecs = updateStartTime.elapsed();
    int channel = 0;

    if (aseb_->sync()) {
        unsigned key = 0;
        for (LabelCheckboxCombo& combo : digitalInputs_) {
            combo.checkbox->setChecked(aseb_->getDigitalInput(key));

            if (combo.select_checkbox->isChecked()) {
                plot->addData(channel, QPointF(msecs, combo.checkbox->isChecked() ? 1 : 0));
                ++channel;
            }
            ++key;
        }

        key = 0;
        for (LabelLineeditCombo& combo : analogInputs_) {
            combo.lineedit->setText(QString("%1").arg(aseb_->getAnalogInput(key)));

            if (combo.select_checkbox->isChecked()) {
                plot->addData(channel, QPointF(msecs, aseb_->getAnalogInput(key)));
                ++channel;
            }
            ++key;
        }
        key = 0;
        for (LabelCheckboxCombo& combo : digitalOutputs_) {
            if (combo.select_checkbox->isChecked()) {
                plot->addData(channel, QPointF(msecs, aseb_->getDigitalOutput(key)?1:0));
                ++channel;
            }
            ++key;
        }

        key = 0;
        for (LabelLineeditCombo& combo : pwmTargets_) {
            if (combo.select_checkbox->isChecked()) {
                plot->addData(channel, QPointF(msecs, aseb_->getTargetPwmOutput(key)));
                ++channel;
            }
        }

        key = 0;
        for (LabelLineeditCombo& combo : pwmOutputs_) {
            float value;
            if(aseb_->getCurrentPwmOutput(key, &value)) {
                combo.lineedit->setText(QString("%1").arg(value));

                if (combo.select_checkbox->isChecked()) {
                    plot->addData(channel, QPointF(msecs, value));
                    ++channel;
                }
            } else {
                combo.lineedit->setText(QString("ERROR"));
            }
            ++key;
        }

        for (LabelLineeditCombo& combo : pwmSpeeds_) {
            if (combo.select_checkbox->isChecked()) {
                plot->addData(channel, QPointF(msecs, combo.lineedit->text().toFloat()));
                ++channel;
            }
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

void FeldbusAsebView::onInputEdited(void) {
    if (updateInterval->hasAcceptableInput() && updateLength->hasAcceptableInput()) {
        updateDuration->setText(QString("Aufzeichnungsfenster: %1 s").arg(updateInterval->text().toFloat() * updateLength->text().toFloat() / 1000.0f));
        updateGraph();
        updateTimer_.start(updateInterval->text().toInt());

    } else {
        updateDuration->setText("");
        updateTimer_.stop();
    }

}

void FeldbusAsebView::updateGraph(void) {
    plot->clear();

    int channel = 0;

    for (LabelCheckboxCombo& combo : digitalInputs_) {
        if (combo.select_checkbox->isChecked()) {
            plot->addChannel(combo.label->text(), static_cast<qreal>(updateLength->text().toFloat() * updateInterval->text().toFloat()));
            ++channel;
        }
    }

    for (LabelLineeditCombo& combo : analogInputs_) {
        if (combo.select_checkbox->isChecked()) {
            plot->addChannel(combo.label->text(), static_cast<qreal>(updateLength->text().toFloat() * updateInterval->text().toFloat()));
            ++channel;
        }
    }

    for (LabelCheckboxCombo& combo : digitalOutputs_) {
        if (combo.select_checkbox->isChecked()) {
            plot->addChannel(combo.label->text(), static_cast<qreal>(updateLength->text().toFloat() * updateInterval->text().toFloat()));
            ++channel;
        }
    }

    for (LabelLineeditCombo& combo : pwmTargets_) {
        if (combo.select_checkbox->isChecked()) {
            plot->addChannel(combo.label->text(), static_cast<qreal>(updateLength->text().toFloat() * updateInterval->text().toFloat()));
            ++channel;
        }
    }

    for (LabelLineeditCombo& combo : pwmOutputs_) {
        if (combo.select_checkbox->isChecked()) {
            plot->addChannel(combo.label->text(), static_cast<qreal>(updateLength->text().toFloat() * updateInterval->text().toFloat()));
            ++channel;
        }
    }

    for (LabelLineeditCombo& combo : pwmSpeeds_) {
        if (combo.select_checkbox->isChecked()) {
            plot->addChannel(combo.label->text(), static_cast<qreal>(updateLength->text().toFloat() * updateInterval->text().toFloat()));
            ++channel;
        }
    }

    updateStartTime.start();
}
