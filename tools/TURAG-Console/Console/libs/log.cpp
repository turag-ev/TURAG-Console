#include "log.h"
#include <QtGlobal>
#include <QMetaMethod>
#include <QMetaObject>


/*
 * ----------------------------------------------
 * class Log
 * ----------------------------------------------
 */

Log::Log(void) :
	model_(new LogModel),
	outputDebugToStderr(true),
	outputDebugToSignals(false),
	outputDebugToModel(true),
	outputLogToStderr(true),
	outputLogToSignals(true),
	outputLogToModel(true),
	threadsafeLogging(false)
{
}

Log::~Log(void) {
	// prevent problems with debug messages from
	// within any destructors that are called
	outputDebugToModel = false;
	delete model_;
}


void Log::captureQtDebugMessages(bool on) {
    if (on) {
        qInstallMessageHandler(messageHandler);
    } else {
        qInstallMessageHandler(0);
    }
}

Log* Log::get(void) {
	static Log instance;
	return &instance;
}

QAbstractItemModel* Log::model(void) {
	return get()->model_;
}

void Log::setDebugOutput(bool outputToModel, bool outputToStderr, bool outputToSignals) {
	get()->outputDebugToModel = outputToModel;
	get()->outputDebugToStderr = outputToStderr;
	get()->outputDebugToSignals = outputToSignals;
}

void Log::setLogOutput(bool outputToModel, bool outputToStderr, bool outputToSignals) {
	get()->outputLogToModel = outputToModel;
	get()->outputLogToStderr = outputToStderr;
	get()->outputLogToSignals = outputToSignals;
}

void Log::setThreadsafeLogging(bool on) {
	get()->threadsafeLogging = on;
}

void Log::messageHandler(QtMsgType type, const QMessageLogContext & context, const QString & msg) {
    QByteArray localMsg = msg.toLocal8Bit();

	Log::MessageType logType;

	switch (type) {
		case QtDebugMsg: logType = Log::MessageType::Debug; break;
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
		case QtInfoMsg: logType = Log::MessageType::Info; break;
#endif
		case QtWarningMsg: logType = Log::MessageType::Warning; break;
		case QtCriticalMsg: logType = Log::MessageType::Critical; break;
		case QtFatalMsg: logType = Log::MessageType::Fatal; break;
	}

	get()->gotMsg(true, logType, context.file, context.line, context.function, localMsg);
}

void Log::gotMsg(bool isDebug, MessageType type, const char * file, int line, const char * function, const QString& msg) {
	if (threadsafeLogging) {
        static QMetaMethod method = get()->metaObject()->method(get()->metaObject()->indexOfSlot("handleGotMsg(bool,MessageType,const char*,int,const char*,QString)"));
        method.invoke(get(), Q_ARG(bool, isDebug), Q_ARG(MessageType, type), Q_ARG(const char*, file), Q_ARG(int, line), Q_ARG(const char*, function), Q_ARG(QString, msg));
	} else {
		handleGotMsg(isDebug, type, file, line, function, msg);
	}
}

// not thread-safe. Should only be called from the thread containing the instance.
void Log::handleGotMsg(bool isDebug, MessageType type, const char* file, int line, const char* function, const QString& msg) {
		// add message to model
	if ((isDebug && outputDebugToModel) || (!isDebug && outputLogToModel)) {
		get()->model_->appendLogMessage(type, msg, QStringLiteral("%1: %2").arg(file).arg(line));
	}

	// print message to stderr
	if ((isDebug && outputDebugToStderr) || (!isDebug && outputLogToStderr)) {
		const char* typeString;
		switch (type) {
			case MessageType::Debug:    typeString = "Debug:"; break;
			case MessageType::Info:     typeString = "Info:"; break;
			case MessageType::Warning:  typeString = "Warning:"; break;
			case MessageType::Critical: typeString = "Critical:"; break;
			case MessageType::Fatal:    typeString = "Fatal:"; break;
		}

		if (file) {
            fprintf(stderr, "%s %s (%s:%u, %s)\n", typeString, msg.toUtf8().constData(), file, line, function);
		} else {
            fprintf(stderr, "%s %s\n", typeString, msg.toUtf8().constData());
		}
	}

	// emit signal
	if ((isDebug && outputDebugToSignals) || (!isDebug && outputLogToSignals)) {
		switch (type) {
			case MessageType::Debug:    emit debugMsgAvailable(msg); break;
			case MessageType::Info:     emit infoMsgAvailable(msg); break;
			case MessageType::Warning:  emit warningMsgAvailable(msg); break;
			case MessageType::Critical: emit criticalMsgAvailable(msg); break;
			case MessageType::Fatal:    emit fatalMsgAvailable(msg); break;
		}
	}
}

void Log::info(const char *file, int line, const char *function, const QString& msg) {
    get()->gotMsg(false, MessageType::Info, file, line, function, msg);
}

void Log::warning(const char *file, int line, const char *function, const QString& msg) {
    get()->gotMsg(false, MessageType::Warning, file, line, function, msg);
}

void Log::critical(const char *file, int line, const char *function, const QString& msg) {
    get()->gotMsg(false, MessageType::Critical, file, line, function, msg);
}





/*
 * ----------------------------------------------
 * class LogModel
 * ----------------------------------------------
 */
LogModel::LogModel(QObject *parent) :
	QAbstractTableModel(parent), logData()
{
}

LogModel::~LogModel(void) { }


int LogModel::rowCount(const QModelIndex &) const {
	return logData.size();
}

int LogModel::columnCount(const QModelIndex &) const {
	return 4;
}

QVariant LogModel::data(const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	int column = index.column();
	int row = index.row();

	if (row >= logData.size() || row < 0)
		return QVariant();

	if (role == Qt::DisplayRole) {
		switch (column) {
			case 0: return logData[row].time.toString();
			case 1:
				switch (logData[row].type) {
					case Log::MessageType::Debug: return QStringLiteral("Debug");
					case Log::MessageType::Info: return QStringLiteral("Info");
					case Log::MessageType::Warning: return QStringLiteral("Warning");
					case Log::MessageType::Critical: return QStringLiteral("Critical");
					case Log::MessageType::Fatal: return QStringLiteral("Fatal");
					default: return QVariant();
				}
			case 2: return logData[row].msg;
			case 3: return logData[row].context;
			default: return QVariant();
		}
	}
	return QVariant();
}


QVariant LogModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (role !=  Qt::DisplayRole || orientation != Qt::Horizontal) {
		return QVariant();
	}

	switch (section) {
		case 0: return QStringLiteral("Zeit");
		case 1: return QStringLiteral("Typ");
		case 2: return QStringLiteral("Meldung");
		case 3: return QStringLiteral("Quelle");
	}

	return QVariant();
}

void LogModel::appendLogMessage(Log::MessageType type, const QString& msg, const QString& context, QTime time) {
	beginInsertRows(QModelIndex(), logData.size(), logData.size());
	logData.append(LogData{type, msg, time, context});
	endInsertRows();
}

void LogModel::appendLogMessage(Log::MessageType type, const QString& msg, const QString& context) {
	beginInsertRows(QModelIndex(), logData.size(), logData.size());
	logData.append(LogData{type, msg, QTime::currentTime(), context});
	endInsertRows();
}

void LogModel::clear(void) {
	beginRemoveRows(QModelIndex(), 0, logData.size() - 1);
	logData.clear();
	endRemoveRows();
}






