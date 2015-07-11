#ifndef LOGGERMODEL_H
#define LOGGERMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QTime>
#include <QString>
#include <QStringRef>
#include <QByteArray>
#include <QLatin1String>
#include <QTextStream>



class LogModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum class MessageType {
        Debug,
        Info,
        Warning,
        Critical,
        Fatal
    };

    explicit LogModel(QObject *parent = 0);
    ~LogModel(void);

    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void appendLogMessage(MessageType type, const QString& msg, const QString& context, QTime time);
    void appendLogMessage(MessageType type, const QString& msg, const QString& context);
    void clear(void);

private:
    struct LogData {
        MessageType type;
        QString msg;
        QTime time;
        QString context;
    };

	QList<LogData> logData;
};


/**
 * @brief Vereinheitlicht die Ausgabe von Debug-Informationen
 * und Fehlermeldungen.
 *
 * Diese Klasse sammelt Debugmeldungen sowie an den Benutzer gerichtete Ausgaben in
 * einem Modell. Dazu übernimmt sie die Ausgabe von Debugmeldungen, die über qDebug(),
 * qWarning(), qCritical() oder qFatal() ausgegeben werden und fügt Informationen über
 * Zeitpunkt und Ort des Auftretens hinzu.
 *
 * Für die Ausgabe von für den Benutzer vorgesehenen Meldungen gibt es die statischen Funktionen
 * info(), warning(), critical() und fatal(). Die über diese Funktionen ausgegebenen Meldungen
 * werden ebenfalls im Log gespeichert und weiterhin über Signale ausgegeben. Die Anwendung
 * kann dann in verbundenen Slots die Meldungen auf passende Art und Weise in der GUI anzeigen.
 *
 * Die Gesamtheit aller Meldungen kann mit einem Model-View an passender Stelle angeboten werden.
 */
class Log : public QObject
{
    Q_OBJECT

public:  
	/**
	 * @brief Aktiviert oder deaktiviert die Verarbeitung von Debug-Nachrichten.
	 * @param on Aktivieren oder deaktivieren.
	 *
	 * Wird die Verarbeitung aktiviert, so werden Nachrichten die mit qDebug(), qWarning(),
	 * qCritical() oder qFatal() ausgegeben werden, um Informationen erweitert auf der
	 * Standardausgabe ausgegeben, sowie im Log gespeichert.
	 *
	 * Wird qFatal() benutzt, so wird das Programm mit abort() beendet.
	 */
    static void captureQtDebugMessages(bool on);

	/**
	 * @brief Gibt das Modell zur Anzeige der Logmeldungen
	 * in einem View zurück.
	 * @return Datenmodell, welches die Logdaten enthält.
	 */
    static QAbstractItemModel* model(void);

	/**
	 * @brief Gibt einen Pointer auf die Instanz zurück.
	 * @return Pointer auf die Log-Instanz.
	 */
    static Log* get(void);

	/**
	 * @brief Gibt eine Info-Meldung für den Benutzer aus.
	 * @param msg Info-Nachricht.
	 */
    static void info(const QString& msg);

	/**
	 * @brief Gibt dem Benutzer eine Warnmeldung aus.
	 * @param msg Warnmeldung.
	 */
    static void warning(const QString& msg);

	/**
	 * @brief Informiert den Benutzer über den Auftritt eines kritischen Fehlers.
	 * @param msg Fehlertext.
	 */
    static void critical(const QString& msg);

	/**
	 * @brief Informiert den Benutzer über den Auftritt eines fatalen Fehlers.
	 * @param msg Fehlertext.
	 */
    static void fatal(const QString& msg);

signals:
	/**
	 * @brief Signalisiert den Auftritt einer Info-Nachricht.
	 * @param msg Infotext.
	 */
    void infoMsgAvailable(const QString& msg);

	/**
	 * @brief Signalisiert den Auftritt einer Warnung.
	 * @param msg Warnungstext.
	 */
    void warningMsgAvailable(const QString& msg);

	/**
	 * @brief Signalisiert den Auftritt eines kritischen Fehlers.
	 * @param msg Beschreibung des Fehlers.
	 */
    void criticalMsgAvailable(const QString& msg);

	/**
	 * @brief Signalisiert den Auftritt eines fatalen Fehlers.
	 * @param msg Beschreibung des Fehlers.
	 */
    void fatalMsgAvailable(const QString& msg);

private:
    explicit Log(void);
    ~Log(void);

    void emitInfoMsgAvailable(const QString& msg);
    void emitWarningMsgAvailable(const QString& msg);
    void emitCriticalMsgAvailable(const QString& msg);
    void emitFatalMsgAvailable(const QString& msg);

    static void messageHandler(QtMsgType, const QMessageLogContext &, const QString &);

	LogModel model_;
};




#endif // LOGGERMODEL_H
