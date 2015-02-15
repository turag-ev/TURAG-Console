#ifndef FELDBUSDEVICE_H
#define FELDBUSDEVICE_H

#include <QObject>
#include <QString>
#include <QTimer>

class QSerialPort;

#define BAUDRATE 500000


class FeldbusDevice : public QObject {
    Q_OBJECT

public:
    explicit FeldbusDevice(QObject* parent = nullptr);
    ~FeldbusDevice(void);
    bool init(QString port);

public slots:
    void dataReceived(void);
    void dataSend(qint64 size);
    void dataRegisterEmpty(void);

public:
    QSerialPort* port;
    QByteArray sendBuffer;
    bool generateDataRegisterEmptyInterrupt;
    QTimer uptimeCounter;
    QTimer processingTimer;


};


#endif // FELDBUSDEVICE_H

