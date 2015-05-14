#ifndef BASEBACKEND_H
#define BASEBACKEND_H

#include <memory>
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QTimer>
#include <QScopedPointer>

class QAction;
class QIODevice;

class BaseBackend : public QObject {
    Q_OBJECT

public:
	explicit BaseBackend(std::initializer_list<QString> protocolScheme, QObject *parent = 0);
	virtual ~BaseBackend(void);

	virtual bool isOpen(void) const;
    virtual bool isReadOnly(void) const;
    virtual QString getConnectionInfo();
    virtual QList<QAction*> getMenuEntries();
	QIODevice* getDevice() { return stream_.data(); }

    // Checks if backend is capable for this connection
    // normally only checks url prefix. This function is thread-safe
    // and so should be any redefinitions.
	bool canHandleUrl(const QString& url) const;

    void setDeviceRecovery(bool on);

    // Save the current contents of the data buffer to the given file.
    // This function is thread-safe.
    void saveBufferToFile(QString fileName);

	// it is potentially dangerous set the chunk size from
	// non-zero to zero when data was already received.
	void setDataEmissionChunkSize(int size) {
		dataEmissionChunkSize = size;
	}

signals:
    // data was received from the backend
    void dataReady(QByteArray data);
	void connected(bool readOnly);
    void disconnected(bool reconnecting);


public slots:
    // reopens a previously closed connection by passing the saved connectionString to
    // `openConnection(QString connectionString)'
    virtual bool openConnection(void);

    // this function needs to open a stream, if possible, with the provided connectionString
    // Further it has to emit the appropriate `connected'-signal.
    virtual bool openConnection(QString connectionString) = 0;

    // closes a connection and emits `disconnected'-signal
    virtual void closeConnection(void);

    // call this function as a response to a loss of connection rather than closeConnection().
    // This will trigger the auto reconnect feature, if enabled.
    virtual void connectionWasLost(void);

    // writes data to opened stream. Assumes that all data can be written. Emits error messages otherwise.
    virtual void writeData(QByteArray data);

    // re-emits the contents of the data buffer
    virtual void reloadData(void);

protected slots:
    // class this function after successfully opening a connection
    void emitConnected(void);

    // Connect this slot to the the readyRead-signal of your data source
    void emitDataReady(void);

    // you should rather use these functions as opposed to directly
    // printing output messages as this would inhibit the message
    // filtering of the base class
    void logFilteredErrorMsg(QString msg);
    void logFilteredInfoMessage(QString msg);

    bool saveBufferToFileInternal(QString fileName);

protected:
	// QScopedPointerDeleteLater is safer for some sub classes of
	// QIODevice
	QScopedPointer<QIODevice, QScopedPointerDeleteLater> stream_;
    QString connectionString_;
	QList<QString> protocolScheme_;
    QByteArray* buffer;

private slots:
    void onRecoverDevice(void);
	void onReadData(void);

private:
    QTimer recoverDeviceTimer;
    QString deviceShouldBeConnectedString;
    bool deviceRecoveryActive;

	QTimer readTimer;
	int readIndex;

	int dataEmissionChunkSize;
};

#endif // BASEBACKEND_H
