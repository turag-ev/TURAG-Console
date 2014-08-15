#include "log.h"
#include <QtGlobal>


LogModel::LogModel(QObject *parent) :
    QAbstractTableModel(parent), logData(new QList<LogData>)
{
}

LogModel::~LogModel(void) {
    delete logData;
}


int LogModel::rowCount(const QModelIndex &) const {
    return logData->size();
}

int LogModel::columnCount(const QModelIndex &) const {
    return 4;
}

QVariant LogModel::data(const QModelIndex & index, int role) const {
    if (!index.isValid())
        return QVariant();

    int column = index.column();
    int row = index.row();

    if (row >= logData->size() || row < 0)
        return QVariant();

    if (role == Qt::DisplayRole) {
        switch (column) {
        case 0: return logData->at(row).time.toString();
        case 1:
            switch (logData->at(row).type) {
            case MessageType::Debug: return QString("Debug");
            case MessageType::Info: return QString("Info");
            case MessageType::Warning: return QString("Warning");
            case MessageType::Critical: return QString("Critical");
            case MessageType::Fatal: return QString("Fatal");
            default: return QVariant();
            }
        case 2: return logData->at(row).msg;
        case 3: return logData->at(row).context;
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
    case 0: return QString("Zeit");
    case 1: return QString("Typ");
    case 2: return QString("Meldung");
    case 3: return QString("Quelle");
    }

    return QVariant();
}

void LogModel::appendLogMessage(MessageType type, const QString& msg, const QString& context, QTime time) {
    beginInsertRows(QModelIndex(), logData->size(), logData->size());
    logData->append(LogData{type, msg, time, context});
    endInsertRows();
}

void LogModel::appendLogMessage(MessageType type, const QString& msg, const QString& context) {
    beginInsertRows(QModelIndex(), logData->size(), logData->size());
    logData->append(LogData{type, msg, QTime::currentTime(), context});
    endInsertRows();
}

void LogModel::clear(void) {
    beginRemoveRows(QModelIndex(), 0, logData->size() - 1);
    logData->clear();
    endRemoveRows();
}



Log::Log(void) :
    model_(new LogModel)
{
}

Log::~Log(void) {
    delete model_;
}


void Log::captureQtDebugMessages(bool on) {
    if (on) {
        qInstallMessageHandler(messageHandler);
    } else {
        qInstallMessageHandler(0);
    }
}

QAbstractItemModel* Log::model(void) {
    return get()->model_;
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
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
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

    get()->model_->appendLogMessage(logType, msg, QString("%1: %2").arg(context.file).arg(context.line));
}


void Log::info(const QString& msg) {
    get()->model_->appendLogMessage(LogModel::MessageType::Info, msg, QString());
    get()->emitInfoMsgAvailable(msg);
}
void Log::warning(const QString& msg) {
    get()->model_->appendLogMessage(LogModel::MessageType::Warning, msg, QString());
    get()->emitWarningMsgAvailable(msg);
}
void Log::critical(const QString& msg) {
    get()->model_->appendLogMessage(LogModel::MessageType::Critical, msg, QString());
    get()->emitCriticalMsgAvailable(msg);
}
void Log::fatal(const QString& msg) {
    get()->model_->appendLogMessage(LogModel::MessageType::Fatal, msg, QString());
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



