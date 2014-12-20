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



class Log : public QObject
{
    Q_OBJECT

public:  
    static void captureQtDebugMessages(bool on);

    static QAbstractItemModel* model(void);
    static Log* get(void);

    static void info(const QString& msg);
    static void warning(const QString& msg);
    static void critical(const QString& msg);
    static void fatal(const QString& msg);

signals:
    void infoMsgAvailable(const QString& msg);
    void warningMsgAvailable(const QString& msg);
    void criticalMsgAvailable(const QString& msg);
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
