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


class LogModel;

/**
 * @brief Vereinheitlicht die Ausgabe von Debug-Informationen
 * und Fehlermeldungen.
 *
 * Grundsätzlich wird zwischen Debugmeldungen und Logmeldungen unterschieden. Erstere
 * werden von den Qt-Makros qDebug(), qInfo(), qWarning(), qCritical() oder qFatal()
 * erzeugt, Logmeldungen durch die Makros logInfo(), logWarning() und logCritical().
 *
 * Sämtliche Ausgaben werden von dieser Klasse gesammelt und bis zu 3 Ausgabekanälen
 * zugeführt:
 * - LogModel-Instanz
 * - stderr
 * - Emission über spezifische Signale
 *
 * Die Ausgabekanäle sind konfigurierbar. Standardmäßig gilt:
 * - LogModel: Debug- und Logmeldungen
 * - stderr: Debug- und Logmeldungen
 * - Signale: Logmeldungen
 *
 * Damit Debugmeldungen erfasst werden können, muss die Funktion
 * captureQtDebugMessages() aufgerufen werden.
 *
 * Zur vereinfachten Erzeugung von Logmeldungen existieren die Makros
 * logInfo(), logWarning() und logCritical().
 *
 * Wird die Log-Klasse in einer Multithread-Anwendung benutzt, so sollte
 * die Funktion setThreadsafeLogging() aufgerufen werden und zwar im Kontext
 * desjenigen Threads, in dem die Log-Instanz existieren soll. Dies muss
 * vor der ersten Verwendung der Klasse geschehen, damit die Instanz innerhalb
 * des korrekten Threads erzeugt wird. Von diesem Thread aus kann dann auch auf
 * das Modell zugegriffen werden und Logmeldungen werden direkt verarbeitet.
 * Werden Logmeldungen von einem anderen Thread erzeugt, so werden diese
 * über die EventQueue verarbeitet.
 *
 * Das thread-sichere Verhalten ist aus Performancegründen standardmäßig
 * deaktiviert.
 */
class Log : public QObject
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

	/**
	 * @brief Aktiviert oder deaktiviert die Verarbeitung von Debug-Nachrichten.
	 * @param on Aktivieren oder deaktivieren.
	 *
	 * Aktiviert die Verarbeitung von Debugnachrichten, die mit qDebug(), qInfo(), qWarning(),
	 * qCritical() oder qFatal() erzeugt werden.
	 */
    static void captureQtDebugMessages(bool on);

	/**
	 * @brief Gibt einen Pointer auf die Instanz zurück.
	 * @return Pointer auf die Log-Instanz.
	 */
	static Log* get(void);

	/**
	 * @brief Gibt das Modell zur Anzeige der Logmeldungen
	 * in einem View zurück.
	 * @return Datenmodell, welches die Logdaten enthält.
	 */
    static QAbstractItemModel* model(void);

	static void setDebugOutput(bool outputToModel, bool outputToStderr, bool outputToSignals);
	static void setLogOutput(bool outputToModel, bool outputToStderr, bool outputToSignals);

	static void setThreadsafeLogging(bool on);

	/**
	 * @brief Gibt eine Info-Meldung für den Benutzer aus.
	 * @param file Name der Quelldatei, die die Meldung erzeugt.
	 * @param line Zeilennumer.
	 * @param function Info über die Funktion, die die Meldung erzeugt.
	 * @param msg Info-Nachricht.
	 */
	static void info(const char * file, int line, const char * function, const QString& msg);

	/**
	 * @brief Gibt dem Benutzer eine Warnmeldung aus.
	 * @param file Name der Quelldatei, die die Meldung erzeugt.
	 * @param line Zeilennumer.
	 * @param function Info über die Funktion, die die Meldung erzeugt.
	 * @param msg Warnmeldung.
	 */
	static void warning(const char * file, int line, const char * function, const QString& msg);

	/**
	 * @brief Informiert den Benutzer über den Auftritt eines kritischen Fehlers.
	 * @param file Name der Quelldatei, die die Meldung erzeugt.
	 * @param line Zeilennumer.
	 * @param function Info über die Funktion, die die Meldung erzeugt.
	 * @param msg Fehlertext.
	 */
	static void critical(const char * file, int line, const char * function, const QString& msg);


signals:
	/**
	 * @brief Signalisiert den Auftritt einer Debug-Nachricht.
	 * @param msg Debugtext.
	 */
	void debugMsgAvailable(const QString& msg);

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


private slots:
    void handleGotMsg(bool isDebug, MessageType type, const char * file, int line, const char * function, const QString& msg);

private:
    explicit Log(void);
    ~Log(void);

    static void messageHandler(QtMsgType, const QMessageLogContext &, const QString &);
    void gotMsg(bool isDebug, MessageType type, const char * file, int line, const char * function, const QString &msg);

	LogModel* model_;

	bool outputDebugToStderr;
	bool outputDebugToSignals;
	bool outputDebugToModel;

	bool outputLogToStderr;
	bool outputLogToSignals;
	bool outputLogToModel;

	bool threadsafeLogging;
};


#define logInfo(msg) Log::info(__FILE__, __LINE__, Q_FUNC_INFO, msg)
#define logWarning(msg) Log::warning(__FILE__, __LINE__, Q_FUNC_INFO, msg)
#define logCritical(msg) Log::critical(__FILE__, __LINE__, Q_FUNC_INFO, msg)



class LogModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	explicit LogModel(QObject *parent = 0);
	~LogModel(void);

	int rowCount(const QModelIndex & parent = QModelIndex()) const override;
	int columnCount(const QModelIndex & parent = QModelIndex()) const override;
	QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	void appendLogMessage(Log::MessageType type, const QString& msg, const QString& context, QTime time);
	void appendLogMessage(Log::MessageType type, const QString& msg, const QString& context);
	void clear(void);

private:
	struct LogData {
		Log::MessageType type;
		QString msg;
		QTime time;
		QString context;
	};

	QList<LogData> logData;
};





#endif // LOGGERMODEL_H
