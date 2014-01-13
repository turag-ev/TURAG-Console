#ifndef FELDBUSASEBVIEW_H
#define FELDBUSASEBVIEW_H

#include <QWidget>
#include <tina++/feldbus/host/aseb.h>
#include <QTimer>
#include <QList>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>

class QLabel;
class QPushButton;


using namespace TURAG::Feldbus;



class FeldbusAsebView : public QWidget
{
    Q_OBJECT

protected:
    struct LabelCheckboxCombo {
        QLabel* label;
        QCheckBox* checkbox;

        LabelCheckboxCombo() : label(nullptr), checkbox(nullptr) {}
    };

    struct LabelLineeditCombo {
        QLabel* label;
        QLineEdit* lineedit;

        LabelLineeditCombo() : label(nullptr), lineedit(nullptr) {}
    };


    QList<LabelCheckboxCombo> digitalInputs_;
    QList<LabelCheckboxCombo> digitalOutputs_;
    QList<LabelLineeditCombo> analogInputs_;
    QList<LabelLineeditCombo> pwmOutputs_;



    Aseb* aseb_;
    Aseb::Analog_t* asebAnalogInputSet_;
    Aseb::Pwm_t* asebPwmOutputSet_;
    uint8_t* asebSyncBuffer_;

    QPushButton* readDevice_;
    QPushButton* resetOutputs_;
    QPushButton* setOutputs_;

    QTimer updateTimer_;

    bool deviceWasRead_;

public:
    explicit FeldbusAsebView(Aseb* aseb, QWidget *parent = 0);

signals:

protected slots:
    void onReadDevice(void);
    void onResetOutputs(void);
    void onSetOutputs(void);

};

#endif // FELDBUSASEBVIEW_H
