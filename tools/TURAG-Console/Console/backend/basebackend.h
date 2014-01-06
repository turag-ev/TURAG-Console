#ifndef BASEBACKEND_H
#define BASEBACKEND_H

#include <QObject>
#include <memory>
#include <QString>
#include <QIODevice>
#include <QByteArray>
#include <QList>
#include <QAction>

class BaseBackend : public QObject {
    Q_OBJECT

protected:
    std::unique_ptr<QIODevice> stream_;
    QString connectionString_;
    const QString connectionPrefix_;

    void emitConnected(void) { emit connected(isReadOnly(), stream_->isSequential()); }

public:
    explicit BaseBackend(QString connectionPrefix, QObject *parent = 0);
    ~BaseBackend(void);

    bool isOpen(void) const;
    virtual bool isReadOnly(void) const;
    bool isSequential(void) const;
    virtual QString getConnectionInfo();
    virtual QList<QAction*> getMenuEntries();
    QIODevice* getDevice() { return stream_.get(); }

    // checks if backend is capable for this connection
    // normally only checks url prefix
    virtual bool canHandleUrl(const QString& url) const;

signals:
    // data was received from the backend
    void dataReady(QByteArray data);
    void connected(bool readOnly, bool isSequential);
    void disconnected(void);
    void errorOccured(QString msg);
    void infoMessage(QString msg);


public slots:
    // reopens a previously closed connection by passing the saved connectionString to
    // `openConnection(QString connectionString)'
    virtual bool openConnection(void);

    // this function needs to open a stream, if possible, with the provided connectionString
    // and has to build the signal-slot connection between stream and backend.
    // Further it has to emit the appropriate `connected'-signal.
    virtual bool openConnection(QString connectionString) = 0;

    // closes a connection, disconnects signals to frontend and emits `disconnected'-signal to controller
    virtual void closeConnection(void);

    // writes data to opened stream. Assumes that all data can be written. Emits error messages otherwise.
    virtual void writeData(QByteArray data);

    // checks for data and, if data is available, results in the emission of a dataReady-Signal.
    // returns all data for non-sequential devices.
    virtual void checkData(void);



};

#endif // BASEBACKEND_H
