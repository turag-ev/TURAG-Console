#ifndef TCPBACKEND_H
#define TCPBACKEND_H

#include "backend/basebackend.h"
#include "backend/tcpBackend_protocol.h"

#include <QString>

class TcpBackend : public BaseBackend {
    Q_OBJECT

protected:

protected slots:


public:
    TcpBackend(QObject *parent);

    static const QString connectionPrefix;

public slots:

};

#endif // TCPBACKEND_H
