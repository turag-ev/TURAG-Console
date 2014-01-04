#ifndef FELDBUSAKTORVIEW_H
#define FELDBUSAKTORVIEW_H

#include <QWidget>
#include <tina++/feldbus/host/aktor.h>
#include <frontend/util/datagraph.h>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QList>
#include <QTime>


class QRadioButton;
class QIntValidator;
class QTimer;
class QGridLayout;
class QSignalMapper;


using namespace TURAG::Feldbus;



class FeldbusAktorView : public QWidget
{
    Q_OBJECT

private:
    void validateInput(void);

protected:
    struct CommandsetEntry {
        uint8_t key;
        QLabel* label;
        QLineEdit* value;
        QPushButton* button;
        QCheckBox* checkbox;
    };

    Aktor* actor;

    DataGraph* plot;
    QPushButton* getCommandSet;
    QPushButton* updateDeviceValues;
    QRadioButton* cyclicDataUpdate;
    QRadioButton* oneShotDataUpdate;
    QPushButton* startStopDataUpdate;
    QLineEdit* updateInterval;
    QLineEdit* updateLength;
    QIntValidator* intervalValidator;
    QIntValidator* lengthValidator;
    QLabel* updateDuration;
    QTimer* updateTimer;
    QList<CommandsetEntry> commandsetGrid;
    AktorCommand_t* commandset;
    QGridLayout* value_grid;
    QSignalMapper* setMapper;
    QTime updateStartTime;

    void readSettings();
    void writeSettings();
    void enableCheckboxes(void);
    void disableCHeckboxes(void);

    unsigned updateCounter;

protected slots:
    void onGetCommandSet(void);
    void onUpdateDeviceValues(void);
    void onCyclicDataUpdate(bool);
    void onOneShotDataUpdate(bool);
    void onStartStopDataUpdate(void);
    void onInputEdited(void);
    void onTimeout(void);
    void onValueSet(int id);
    void onCheckboxChanged(void);

public:
    explicit FeldbusAktorView(Aktor* aktor, QWidget *parent = 0);
    ~FeldbusAktorView(void);

signals:

public slots:

};

#endif // FELDBUSAKTORVIEW_H
