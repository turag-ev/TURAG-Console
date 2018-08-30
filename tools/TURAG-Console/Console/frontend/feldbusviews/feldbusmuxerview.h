#ifndef FELDBUSMUXERVIEW_H
#define FELDBUSMUXERVIEW_H

#include <QWidget>
#include <QTimer>

#include <tina++/feldbus/host/muxer_64_32.h>


class QLineEdit;
class QPushButton;
class QComboBox;

class FeldbusMuxerView : public QWidget
{
    Q_OBJECT

public:
    explicit FeldbusMuxerView(TURAG::Feldbus::Muxer_64_32* muxer, QWidget *parent = 0);
    ~FeldbusMuxerView();

protected:
    TURAG::Feldbus::Muxer_64_32* muxer;

    QPushButton* outputEnabled;
    QLineEdit* triggerModeEdit;
    QComboBox* triggerModeComboBox;

    QLineEdit* maxCycleLengthEdit;
    QLineEdit* cycleLengthEdit;
    QLineEdit* cycleIndexEdit;
    QLineEdit* changeCycleLengthEdit;
    QLineEdit* changeCycleIndexEdit;

    QLineEdit* setTransmitterEdit;
    QLineEdit* setReceiverEdit;

    QLineEdit* configCycleEdit;
    QLineEdit* configTransmitterEdit;
    QLineEdit* configReceiverEdit;


    QTimer updateTimer;

private slots:
    void SetCycleIndex();
    void SetCycleLength();
    void update();
    void setManualOutput();
    void setOutput();
    void setCycleConfig();
    void clearCycleConfig();
    void setTriggerMode();



private:
    void setReadOnly(QLineEdit* lineEedit);
};

#endif // FELDBUSMUXERVIEW_H
