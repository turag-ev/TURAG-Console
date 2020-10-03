#ifndef TURAG_CONSOLE_CONSOLE_BACKEND_SERIALBACKEND_H
#define TURAG_CONSOLE_CONSOLE_BACKEND_SERIALBACKEND_H

#include "basebackend.h"
#include <QSerialPort>


class SerialBackend : public BaseBackend {
    Q_OBJECT


public:
    static constexpr int MAX_READ_ERRORS = 10;
    
    SerialBackend(QObject *parent = 0);
	virtual ~SerialBackend();

	virtual QString getConnectionInfo() const;

	static const QString protocolScheme;

private slots:
    void onError(QSerialPort::SerialPortError error);

private:
	virtual bool doConnectionPreconditionChecking(const QUrl& url);
	virtual BaseBackend::ConnectionStatus doOpenConnection(const QUrl& connectionUrl);
    
    int readErrorCounter;
};


#endif // TURAG_CONSOLE_CONSOLE_BACKEND_SERIALBACKEND_H
