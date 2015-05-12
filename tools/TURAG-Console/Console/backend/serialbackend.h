#ifndef SERIALBACKEND_H
#define SERIALBACKEND_H

#include "basebackend.h"
#include <QtSerialPort/QSerialPort>


class SerialBackend : public BaseBackend {
    Q_OBJECT


public:
    SerialBackend(QObject *parent = 0);
	virtual ~SerialBackend() {
		closeConnection();
	}

    static const QString protocolScheme;
    virtual QString getConnectionInfo();

protected slots:
    void onError(QSerialPort::SerialPortError error);

public slots:
    virtual bool openConnection(QString connectionString);
};


#endif // SERIALBACKEND_H
