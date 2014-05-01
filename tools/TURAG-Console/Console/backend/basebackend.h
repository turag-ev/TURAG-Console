#ifndef BASEBACKEND_H
#define BASEBACKEND_H

#include <memory>
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QTimer>

class QAction;
class QIODevice;

class BaseBackend : public QObject {
    Q_OBJECT

public:
    explicit BaseBackend(QString connectionPrefix, QObject *parent = 0);
    ~BaseBackend(void);

    bool isOpen(void) const;
    virtual bool isReadOnly(void) const;
    virtual bool isBuffered(void) const;
    virtual QString getConnectionInfo();
    virtual QList<QAction*> getMenuEntries();
    QIODevice* getDevice() { return stream_.get(); }

    // checks if backend is capable for this connection
    // normally only checks url prefix
    virtual bool canHandleUrl(const QString& url) const;

    void setDeviceRecovery(bool on);

signals:
    // data was received from the backend
    void dataReady(QByteArray data);
    void connected(bool readOnly, bool isBuffered);
    void disconnected(bool reconnecting);
    void errorOccured(QString msg);
    void infoMessage(QString msg);


public slots:
    // reopens a previously closed connection by passing the saved connectionString to
    // `openConnection(QString connectionString)'
    virtual bool openConnection(void);

    // this function needs to open a stream, if possible, with the provided connectionString
    // Further it has to emit the appropriate `connected'-signal.
    virtual bool openConnection(QString connectionString) = 0;

    // closes a connection and emits `disconnected'-signal
    virtual void closeConnection(void);

    // call this function as a response to a loss of connection rather than closeConnection()
    virtual void connectionWasLost(void);

    // writes data to opened stream. Assumes that all data can be written. Emits error messages otherwise.
    virtual void writeData(QByteArray data);

    // checks for data and, if data is available, results in the emission of a dataReady-Signal.
    // returns all data for buffered devices.
    virtual void checkData(void);

protected slots:
    // class this function after successfully opening a connection
    void emitConnected(void);

    // you should rather use these functions as opposed to directly
    // emitting the associated signals as this would inhibit the message
    // filtering of the base class
    void emitErrorOccured(QString msg);
    void emitInfoMessage(QString msg);

protected:
    std::unique_ptr<QIODevice> stream_;
    QString connectionString_;
    const QString connectionPrefix_;

private slots:
    void onRecoverDevice(void);

private:
    QTimer recoverDeviceTimer;
    QString deviceShouldBeConnectedString;
    bool deviceRecoveryActive;

};

#endif // BASEBACKEND_H
