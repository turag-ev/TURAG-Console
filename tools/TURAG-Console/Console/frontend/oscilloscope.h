#ifndef OSCILLOSCOPE_H
#define OSCILLOSCOPE_H

#include "basefrontend.h"
#include "util/datagraph.h"
#include <QList>
#include "util/datapointinterface.h"

class QLineEdit;
class QCheckBox;
class QLabel;
class QPointF;
class DataPointInterface;
class QTabWidget;
class QByteArray;


class Oscilloscope : public BaseFrontend {
    Q_OBJECT

protected:
    DataGraph* plot;
    QTabWidget* tab;
    QLineEdit* comma_textbox;
    QLineEdit* delim_textbox;
    QCheckBox* delim_newline;
    QCheckBox* delim_emptyline;
    QLineEdit* subdelim_textbox;
    QCheckBox* subdelim_newline;
    QCheckBox* timechannel_checkbox;
    QLabel* channel_label;
    
    DataPointInterface* interface;
    bool hasSequentialConnection;

    void readSettings();
    void writeSettings();

protected slots:
    void onStreamTypeChanged(int index);
    void onDataPointsReady(QList<DataEntry> dataPoints);
    
    void onTextChannelsDetected(int amount);
    void onTextSettingsChanged();
    void onTextSettingsDelimNewLineChanged(bool checked);
    void onTextSettingsDelimEmptyLineChanged(bool checked);

public:
    explicit Oscilloscope(QWidget *parent = 0);
    ~Oscilloscope();

public slots:
    virtual void writeData(QByteArray data);
    virtual void clear(void);
    virtual bool saveOutput(void);
    virtual void onConnected(bool readOnly, bool isSequential, QIODevice*);
};



#endif // OSCILLOSCOPE_H
