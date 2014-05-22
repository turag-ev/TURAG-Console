#ifndef FELDBUSAKTORVIEW_H
#define FELDBUSAKTORVIEW_H

#include <QWidget>
#include <tina++/feldbus/host/aktor.h>
#include <frontend/graphutils/datagraph.h>
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
class LineEditExt;


using namespace TURAG::Feldbus;



class FeldbusAktorView : public QWidget
{
    Q_OBJECT

public:
    explicit FeldbusAktorView(Aktor* aktor, QWidget *parent = 0);
    ~FeldbusAktorView(void);
    QList<QAction *> getActions(void) { return plot->getActions(); }

signals:

public slots:

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
    LineEditExt* updateInterval;
    LineEditExt* updateLength;
    QLabel* updateDuration;
    QTimer* updateTimer;
    QList<CommandsetEntry> commandsetGrid;
    AktorCommand_t* commandset;
    QSignalMapper* setMapper;
    QTime updateStartTime;

    void readSettings();
    void writeSettings();
    void enableCheckboxes(void);
    void disableCheckboxes(void);

    unsigned updateCounter;

protected slots:
    void onUpdateDeviceValues(void);
    void onStartStopDataUpdate(void);
    void onInputEdited(void);
    void onTimeout(void);
    void onValueSet(int id);
    void onCheckboxChanged(void);
    void onUserInput(void);

private:
    void validateInput(void);

};

#endif // FELDBUSAKTORVIEW_H
