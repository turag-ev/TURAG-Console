#ifndef PROTOCOL_H
#define PROTOCOL_H

/*
 *In dieser Datei werden die Konstanten, die für das Protokoll nötig sind, definiert
 */

/*
 *
 *Hier ist das Datum und die Uhrzeit der letzten Änderung einzutragen!
 *13.12.13
 *22:54
 *
 */

//Hier werden die Namen der Attribute in den XML-Tokens definiert
#define DEVICEPATH QString("path")
#define DEVICEPORT QString("port")
#define DEVICEDESCRIPTION QString("description")
#define DEVICERESETCODE QString("resetcode")
#define DEVICEBAUDRATE QString("baudrate")

//TERMINATE definiert den String, der beim Terminieren eines Datenstreams gesendet wird
#define TERMINATE QString("terminate")

//Schlüsselwort, das devices ankündigt
#define DEVICE QString("device:")

//legt das Schlüsselwort fest, um Schreibrechte anzufordern
#define WRITEACCESS QString("writeAccess")

//Schlüsselwort um Schreibrechte aufzugeben
#define READONLY QString("readOnly")

//Wird gesendet, wenn Schreibrechte erteilt wurden/werden
#define WAGRANTED QString("writeAccessGranted")

//Wirt gesendet, wenn Anfrage auf Schreibrechte abgelehnt wurde
#define WADENIED QString("wirteAccessDenied")

//kündigt das Senden der OnlineStati der devices an
#define ONLINESTATUS QString("onlineStatus")

//legt die maximale Wartezeit [ms] beim Schreiben von Daten (->Client) fest
#define WRITEBYTETIMEOUT 500

//definiert die modes für ControlServer::handleWriteAccess(...)
#define REQUEST QBool (true)
#define DROP QBool (false)

#define RETRYINTERVAL 1000 //setzt das intervall, nach dem das device(offline) neu angesprochen wird
#define MAXREADLENGTH 100 //legt fest, bis zu welcher Länge(Bytes) lines vom Client eingelesen werden

#endif // PROTOCOL_H
