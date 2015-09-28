#ifndef TCPSOCKETEXT_H
#define TCPSOCKETEXT_H


#include <QtNetwork/QTcpSocket>

/**
 * @brief TCP Socket-Klasse, die es ermöglicht das Keep-Alive
 * Feature zu aktivieren und einzustellen.
 */
class TcpSocketExt : public QTcpSocket {
public:
	/**
	 * @brief Konstruktor
	 * @param keepAlive Gibt an, ob Keep-Alive aktiviert wird.
	 * @param keepAliveTime Zeit in Sekunden nach deren Ablauf begonnen wird,
	 * Keep-Alive-Probes zu versenden
	 * @param keepAliveInterval Zeit in Sekunden zwischen versendeten Probes.
	 * @param parent
	 *
	 * Die Anzahl der versendeten Keep-Alive-Probes ist platform-abhängig.
	 * \see http://tldp.org/HOWTO/TCP-Keepalive-HOWTO/usingkeepalive.html
	 * \see https://msdn.microsoft.com/de-de/library/windows/desktop/dd877220%28v=vs.85%29.aspx
	 */
	explicit TcpSocketExt(bool keepAlive = false, int keepAliveTime = 5, int keepAliveInterval = 1, QObject* parent = 0);

};




#endif // TCPSOCKETEXT_H

