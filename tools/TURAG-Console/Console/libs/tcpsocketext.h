#ifndef TCPSOCKETEXT_H
#define TCPSOCKETEXT_H


#include <QtNetwork/QAbstractSocket>

class TcpSocketExt : public QAbstractSocket {
public:
	explicit TcpSocketExt(bool keepAlive = false, int keepAliveTime = 5, int keepAliveInterval = 1, QObject* parent = 0);

};




#endif // TCPSOCKETEXT_H

