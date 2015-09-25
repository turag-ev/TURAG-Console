#include "tcpsocketext.h"
#include <QDebug>



#ifdef _WIN32


TcpSocketExt::TcpSocketExt(bool keepAlive, int keepAliveTime, int keepAliveInterval, QObject *parent) :
	QAbstractSocket(QAbstractSocket::TcpSocket, parent)
{
#error not implemented
}



#else // _WIN32


#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

TcpSocketExt::TcpSocketExt(bool keepAlive, int keepAliveTime, int keepAliveInterval, QObject *parent) :
	QAbstractSocket(QAbstractSocket::TcpSocket, parent)
{
	int s;
	int optval;
	socklen_t optlen = sizeof(optval);

	/* Create the socket */
	if ((s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		qCritical() << "Couldn't create socket";
		return;
	}
	setSocketDescriptor(s, QAbstractSocket::UnconnectedState, QIODevice::NotOpen);

	if (keepAlive) {
		/* Set the option active */
		optval = 1;
		optlen = sizeof(optval);
		if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
		   qWarning() << "Couldn't enable tcp keep alive";
		   return;
		}

		// set keep alive time
		optval = keepAliveTime;
		optlen = sizeof(optval);
		if (setsockopt(s, SOL_TCP, TCP_KEEPIDLE, &optval, optlen) < 0) {
		   qWarning() << "Couldn't set keep alive time";
		   return;
		}

		// set keep alive interval
		optval = keepAliveInterval;
		optlen = sizeof(optval);
		if (setsockopt(s, SOL_TCP, TCP_KEEPINTVL, &optval, optlen) < 0) {
		   qWarning() << "Couldn't set keep alive time";
		   return;
		}
	}
}

#endif // _WIN32

