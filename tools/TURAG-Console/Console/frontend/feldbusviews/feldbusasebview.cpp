#include "feldbusasebview.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>

FeldbusAsebView::FeldbusAsebView(Aseb* aseb, QWidget *parent) :
    QWidget(parent), aseb_(aseb), deviceWasRead_(false)
{
    QHBoxLayout* layout = new QHBoxLayout;
    QVBoxLayout* left_layout = new QVBoxLayout;
    QVBoxLayout* right_layout = new QVBoxLayout;
    QVBoxLayout* digital_in_layout = new QVBoxLayout;
    QVBoxLayout* analog_in_layout = new QVBoxLayout;
    QVBoxLayout* digital_out_layout = new QVBoxLayout;
    QVBoxLayout* pwm_out_layout = new QVBoxLayout;
    QVBoxLayout* button_layout = new QVBoxLayout;

    setLayout(layout);
    layout->addLayout(left_layout);
    layout->addLayout(right_layout);

    QLabel* input_label = new QLabel("Eingänge");
    left_layout->addWidget(input_label);
    left_layout->addLayout(digital_in_layout);
    left_layout->addLayout(analog_in_layout);
    left_layout->setAlignment(input_label, Qt::AlignTop);

    readDevice_ = new QPushButton("Gerät einlesen");
    connect(readDevice_, SIGNAL(clicked()), this, SLOT(onReadDevice()));
    left_layout->addWidget(readDevice_);
    left_layout->setAlignment(readDevice_, Qt::AlignBottom);

    QLabel* output_label = new QLabel("Ausgänge");
    right_layout->addWidget(output_label);
    right_layout->addLayout(digital_out_layout);
    right_layout->addLayout(pwm_out_layout);
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

}


void FeldbusAsebView::onReadDevice(void) {
    if (deviceWasRead_) {

    }
}

void FeldbusAsebView::onResetOutputs(void) {

}

void FeldbusAsebView::onSetOutputs(void) {

}
