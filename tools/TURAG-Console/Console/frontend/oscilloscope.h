#ifndef OSCILLOSCOPE_H
#define OSCILLOSCOPE_H

#include "basefrontend.h"
#include "util/datagraph.h"
#include <QList>
#include "util/datapointinterface.h"

class LineEditExt;
class CheckBoxExt;
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
    LineEditExt* comma_textbox;
    LineEditExt* delim_textbox;
    CheckBoxExt* delim_newline;
    CheckBoxExt* delim_emptyline;
    LineEditExt* subdelim_textbox;
    CheckBoxExt* subdelim_newline;
    CheckBoxExt* timechannel_checkbox;
    QLabel* channel_label;
    
    DataPointInterface* interface;
    bool hasBufferedConnection;

    void readSettings();

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
    virtual bool saveOutput(QString file);
    virtual void onConnected(bool readOnly, bool isBuffered, QIODevice*);
};



#endif // OSCILLOSCOPE_H
