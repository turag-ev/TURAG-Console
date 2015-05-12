#include "log.h"
#include <QtGlobal>


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
			case MessageType::Debug: return QStringLiteral("Debug");
			case MessageType::Info: return QStringLiteral("Info");
			case MessageType::Warning: return QStringLiteral("Warning");
			case MessageType::Critical: return QStringLiteral("Critical");
			case MessageType::Fatal: return QStringLiteral("Fatal");
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

void LogModel::appendLogMessage(MessageType type, const QString& msg, const QString& context, QTime time) {
	beginInsertRows(QModelIndex(), logData.size(), logData.size());
	logData.append(LogData{type, msg, time, context});
    endInsertRows();
}

void LogModel::appendLogMessage(MessageType type, const QString& msg, const QString& context) {
	beginInsertRows(QModelIndex(), logData.size(), logData.size());
	logData.append(LogData{type, msg, QTime::currentTime(), context});
    endInsertRows();
}

void LogModel::clear(void) {
	beginRemoveRows(QModelIndex(), 0, logData.size() - 1);
	logData.clear();
    endRemoveRows();
}



Log::Log(void) :
	model_()
{
}

Log::~Log(void) {
}


void Log::captureQtDebugMessages(bool on) {
    if (on) {
        qInstallMessageHandler(messageHandler);
    } else {
        qInstallMessageHandler(0);
    }
}

QAbstractItemModel* Log::model(void) {
	return &(get()->model_);
}

Log* Log::get(void) {
    static Log instance;

    return &instance;
}

void Log::messageHandler(QtMsgType type, const QMessageLogContext & context, const QString & msg) {

    // print debug message to stderr
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
		if (context.file) {
			fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		} else {
			fprintf(stderr, "Debug: %s\n", localMsg.constData());
		}
		break;
    case QtWarningMsg:
		if (context.file) {
			fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		} else {
			fprintf(stderr, "Warning: %s\n", localMsg.constData());
		}
		break;
    case QtCriticalMsg:
		if (context.file) {
			fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		} else {
			fprintf(stderr, "Critical: %s\n", localMsg.constData());
		}
		break;
    case QtFatalMsg:
		if (context.file) {
			fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		} else {
			fprintf(stderr, "Fatal: %s\n", localMsg.constData());
		}
		abort();
    }

    // pipe debug message to log model
    LogModel::MessageType logType;

    switch (type) {
    case QtDebugMsg: logType = LogModel::MessageType::Debug; break;
    case QtWarningMsg: logType = LogModel::MessageType::Warning; break;
    case QtCriticalMsg: logType = LogModel::MessageType::Critical; break;
    case QtFatalMsg: logType = LogModel::MessageType::Fatal; break;
    }

	get()->model_.appendLogMessage(logType, msg, QStringLiteral("%1: %2").arg(context.file).arg(context.line));
}


void Log::info(const QString& msg) {
	get()->model_.appendLogMessage(LogModel::MessageType::Info, msg, QString());
    get()->emitInfoMsgAvailable(msg);
}
void Log::warning(const QString& msg) {
	get()->model_.appendLogMessage(LogModel::MessageType::Warning, msg, QString());
    get()->emitWarningMsgAvailable(msg);
}
void Log::critical(const QString& msg) {
	get()->model_.appendLogMessage(LogModel::MessageType::Critical, msg, QString());
    get()->emitCriticalMsgAvailable(msg);
}
void Log::fatal(const QString& msg) {
	get()->model_.appendLogMessage(LogModel::MessageType::Fatal, msg, QString());
    get()->emitFatalMsgAvailable(msg);
}


void Log::emitInfoMsgAvailable(const QString& msg) {
    emit infoMsgAvailable(msg);
}

void Log::emitWarningMsgAvailable(const QString& msg) {
    emit warningMsgAvailable(msg);
}

void Log::emitCriticalMsgAvailable(const QString& msg) {
    emit criticalMsgAvailable(msg);
}

void Log::emitFatalMsgAvailable(const QString& msg) {
    emit fatalMsgAvailable(msg);
}



