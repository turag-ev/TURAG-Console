#ifndef TURAG_CONSOLE_CONSOLE_FRONTEND_UTIL_DATAPOINTINTERFACE_H
#define TURAG_CONSOLE_CONSOLE_FRONTEND_UTIL_DATAPOINTINTERFACE_H

#include <QObject>
#include <qwt_system_clock.h>
#include <QList>

class QByteArray;


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
    QByteArray delim_;
    QByteArray channelDelim_;

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
    void setDelim(QByteArray delim);
    void setChannelDelim(QByteArray delim);
    void setFirstChannelIsTime(bool isTime);

signals:
    void channelsDetected(int amount);

};

#endif // TURAG_CONSOLE_CONSOLE_FRONTEND_UTIL_DATAPOINTINTERFACE_H
