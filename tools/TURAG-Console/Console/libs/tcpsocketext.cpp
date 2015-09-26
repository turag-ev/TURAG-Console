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
	QTcpSocket(parent)
{
	if (keepAlive) {
		connect(this, &TcpSocketExt::connected, [this, keepAliveTime, keepAliveInterval]() {
			int enableKeepAlive = 1;
			int fd = socketDescriptor();
			if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &enableKeepAlive, sizeof(enableKeepAlive)) < 0) {
				qWarning() << "Couldn't set socket opt";
				return;
			}

			if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &keepAliveTime, sizeof(keepAliveTime)) < 0) {
				qWarning() << "Couldn't set socket opt";
				return;
			}

			int count = 3; // send up to 3 keepalive packets out, then disconnect if no response
			if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count)) < 0) {
				qWarning() << "Couldn't set socket opt";
				return;
			}

			if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &keepAliveInterval, sizeof(keepAliveInterval)) < 0) {
				qWarning() << "Couldn't set socket opt";
				return;
			}
		});
	}
}


#endif // _WIN32

