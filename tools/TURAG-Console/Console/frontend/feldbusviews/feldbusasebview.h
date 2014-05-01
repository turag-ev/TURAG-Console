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
class QGridLayout;


using namespace TURAG::Feldbus;



class FeldbusAsebView : public QWidget
{
    Q_OBJECT

public:
    explicit FeldbusAsebView(Aseb* aseb, QWidget *parent = 0);
    ~FeldbusAsebView(void);

protected slots:
    void onResetOutputs(void);
    void onSetOutputs(void);
    void onUpdate(void);
    void onUserInput(void);


protected:
    struct LabelCheckboxCombo {
        QLabel* label;
        QCheckBox* checkbox;

        LabelCheckboxCombo() : label(nullptr), checkbox(nullptr) {}
        LabelCheckboxCombo(QLabel* label_, QCheckBox* checkbox_) :
            label(label_), checkbox(checkbox_) {}
    };

    struct LabelLineeditCombo {
        QLabel* label;
        QLineEdit* lineedit;

        LabelLineeditCombo() : label(nullptr), lineedit(nullptr) {}
        LabelLineeditCombo(QLabel* label_, QLineEdit* lineedit_) :
            label(label_), lineedit(lineedit_) {}
    };


    QList<LabelCheckboxCombo> digitalInputs_;
    QList<LabelCheckboxCombo> digitalOutputs_;
    QList<LabelLineeditCombo> analogInputs_;
    QList<LabelLineeditCombo> pwmOutputs_;

    QGridLayout* digital_in_layout_;
    QGridLayout* analog_in_layout_;
    QGridLayout* digital_out_layout_;
    QGridLayout* pwm_out_layout_;


    Aseb* aseb_;
    Aseb::Analog_t* asebAnalogInputSet_;
    Aseb::Pwm_t* asebPwmOutputSet_;
    uint8_t* asebSyncBuffer_;

    QPushButton* resetOutputs_;
    QPushButton* setOutputs_;

    QLabel* syncBufferSize_;
    QLabel* analogInputSize_;
    QLabel* pwmOutputSize_;


    QTimer updateTimer_;

};

#endif // FELDBUSASEBVIEW_H
