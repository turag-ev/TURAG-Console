#ifndef FELDBUSAKTORVIEW_H
#define FELDBUSAKTORVIEW_H

#include <QWidget>
#include <tina++/feldbus/host/aktor.h>
#include <frontend/util/datagraph.h>


class QPushButton;
class QRadioButton;
class QLineEdit;

using namespace TURAG::Feldbus;


class FeldbusAktorView : public QWidget
{
    Q_OBJECT

protected:
    Aktor* actor;

    DataGraph* plot;
    QPushButton* getCommandSet;
    QPushButton* updateDeviceValues;
    QRadioButton* cyclicDataUpdate;
    QRadioButton* oneShotDataUpdate;
    QPushButton* startStopDataUpdate;
    QLineEdit* updateInterval;
    QLineEdit* updateLength;

protected slots:
    void onGetCommandSet(void);
    void onUpdateDeviceValues(void);
    void onCyclicDataUpdate(bool);
    void onOneShotDataUpdate(bool);

public:
    explicit FeldbusAktorView(Aktor* aktor, QWidget *parent = 0);

signals:

public slots:

};

#endif // FELDBUSAKTORVIEW_H
