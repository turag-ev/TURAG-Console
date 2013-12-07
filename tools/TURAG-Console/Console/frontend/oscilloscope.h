#ifndef OSCILLOSCOPE_H
#define OSCILLOSCOPE_H

#include "basefrontend.h"
#include "util/datagraph.h"
#include <QList>
#include <qwt_system_clock.h>

class QLineEdit;
class QCheckBox;
class QLabel;
class QPointF;
class DataPointInterface;
class DataEntry;
class QTabWidget;


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


class DataEntry {
public:
    DataEntry() : channel_(0), time_(0), data_(0) {}
    DataEntry(int channel, float time, float data) : channel_(channel), time_(time), data_(data) {}
    int channel_;
    float time_;
    float data_;
};


class DataPointInterface : public QObject {
    Q_OBJECT
protected:
    QByteArray buffer;
    
public slots:
    virtual void writeData(QByteArray data) = 0;
    virtual void clear() = 0;
    
signals:
    void dataPointsReady(QList<DataEntry> dataPoints);
};


class TextDataPointInterface : public DataPointInterface {
    Q_OBJECT
protected:
    char delim_;
    bool delimIsNewline_;
    bool delimIsEmptyLine_;
    
    char channelDelim_;
    bool ChannelDelimIsNewline_;
    
    char decimalPoint_;
    bool firstChannelIsTime_;
    
    bool detectChannels;
        
    QwtSystemClock clock;


public:
    explicit TextDataPointInterface();
    
public slots:
    virtual void writeData(QByteArray data);
    void clear();
    
    void setDecimalPoint(char decimalPoint);
    void setDelim(char delim);
    void setDelimNewline();
    void setDelimEmptyLine();
    void setChannelDelim(char delim);
    void setChannelDelimNewline();
    void setFirstChannelIsTime(bool isTime);
    
signals:
    void channelsDetected(int amount);
    
};

#endif // OSCILLOSCOPE_H
