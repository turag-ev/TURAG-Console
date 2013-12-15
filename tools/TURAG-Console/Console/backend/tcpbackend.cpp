#include "tcpbackend.h"

const QString SerialBackend::connectionPrefix = "tcp://";

TcpBackend::TcpBackend (QObject *parent) :
    BaseBackend(TcpBackend::connectionPrefix, parent) {

}

