#ifndef FELDBUSASEBVIEW_H
#define FELDBUSASEBVIEW_H

#include <QWidget>
#include <tina++/feldbus/host/aseb.h>
#include <QTimer>
#include <QList>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QTime>
#include <frontend/graphutils/datagraph.h>

class QLabel;
class QPushButton;
class QGridLayout;
class DataGraph;
class LineEditExt;


using namespace TURAG::Feldbus;



class FeldbusAsebView : public QWidget
{
    Q_OBJECT

public:
    explicit FeldbusAsebView(Aseb* aseb, QWidget *parent = 0);
    ~FeldbusAsebView(void);
    QList<QAction *> getActions(void) { return plot->getActions(); }

protected slots:
    void onResetOutputs(void);
    void onSetOutputs(void);
    void onUpdate(void);
    void onUserInput(void);
    void onInputEdited(void);
    void updateGraph(void);


protected:
    struct LabelCheckboxCombo {
        QLabel* label;
        QCheckBox* checkbox;
        QCheckBox* select_checkbox;


        LabelCheckboxCombo() : label(nullptr), checkbox(nullptr), select_checkbox(nullptr) {}
        LabelCheckboxCombo(QLabel* label_, QCheckBox* checkbox_, QCheckBox* select_checkbox_) :
            label(label_), checkbox(checkbox_), select_checkbox(select_checkbox_) {}
    };

    struct LabelLineeditCombo {
        QLabel* label;
        QLineEdit* lineedit;
        QCheckBox* select_checkbox;

        LabelLineeditCombo() : label(nullptr), lineedit(nullptr), select_checkbox(nullptr) {}
        LabelLineeditCombo(QLabel* label_, QLineEdit* lineedit_, QCheckBox* select_checkbox_) :
            label(label_), lineedit(lineedit_), select_checkbox(select_checkbox_) {}
    };


    QList<LabelCheckboxCombo> digitalInputs_;
    QList<LabelCheckboxCombo> digitalOutputs_;
    QList<LabelLineeditCombo> analogInputs_;
    QList<LabelLineeditCombo> pwmOutputs_;

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
    QTime updateStartTime;

    DataGraph* plot;
    LineEditExt* updateInterval;
    LineEditExt* updateLength;
    QLabel* updateDuration;


};

#endif // FELDBUSASEBVIEW_H
