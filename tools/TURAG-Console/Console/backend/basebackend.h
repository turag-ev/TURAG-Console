#ifndef BASEBACKEND_H
#define BASEBACKEND_H

#include <memory>
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QTimer>
#include <QScopedPointer>
#include <QUrl>

class QAction;
class QIODevice;

class BaseBackend : public QObject {
    Q_OBJECT

public:
	explicit BaseBackend(std::initializer_list<QString> protocolScheme, QObject *parent = 0);
	virtual ~BaseBackend(void);

	virtual bool isOpen(void) const;

    virtual bool isReadOnly(void) const;

	virtual bool isConnectionInProgress(void) const;

	// Checks if backend is capable for this connection.
	// Simply checks url scheme.
	bool canHandleUrl(const QUrl& url) const;

	virtual QString getConnectionInfo() const;


	QIODevice* getDevice() { return stream_.data(); }


    void setDeviceRecovery(bool on);

    // Save the current contents of the data buffer to the given file.
    // This function is thread-safe.
    void saveBufferToFile(QString fileName);

	// it is potentially dangerous set the chunk size from
	// non-zero to zero when data was already received.
	void setDataEmissionChunkSize(int size) {
		dataEmissionChunkSize = size;
	}


public slots:
    // reopens a previously closed connection by passing the saved connectionString to
    // `openConnection(QString connectionString)'
	bool openConnection(void);

	// Opens the specified url.
	bool openConnection(const QUrl& connectionUrl);

    // closes a connection and emits `disconnected'-signal
	void closeConnection(void);

    // writes data to opened stream. Assumes that all data can be written. Emits error messages otherwise.
	void writeData(QByteArray data);

    // re-emits the contents of the data buffer
	void reloadData(void);


signals:
	// connection was successfully established
	void connected(bool readOnly);

	// connection was terminated.
	void disconnected(bool reconnecting);

	// data was received from the backend.
	void dataReady(QByteArray data);


protected slots:
	// call this function after successfully opening a connection
	void connectingSuccessful(void);
	void connectingFailed(void);
	// call this function as a response to a loss of connection rather than closeConnection().
	// This will trigger the auto reconnect feature, if enabled.
	void connectionWasLost(void);


    // Connect this slot to the the readyRead-signal of your data source
    void emitDataReady(void);

    // you should rather use these functions as opposed to directly
    // printing output messages as this would inhibit the message
    // filtering of the base class
    void logFilteredErrorMsg(QString msg);
    void logFilteredInfoMessage(QString msg);

    bool saveBufferToFileInternal(QString fileName);

protected:
	enum class ConnectionStatus { failed, successful, ongoing };

	// QScopedPointerDeleteLater is safer for some sub classes of
	// QIODevice
	QScopedPointer<QIODevice, QScopedPointerDeleteLater> stream_;
	QUrl connectionUrl_;
	QList<QString> supportedProtocolSchemes_;
	QByteArray buffer;

	// ???
	static_assert(sizeof(QString) == sizeof(QString*),"");

private slots:
    void onRecoverDevice(void);
	void onReadData(void);

private:
	// Derived classes can reimplement this function if extra checks
	// prior to connecting are required. The base implementation does
	// nothing. This function is called before an old connection is
	// closed.
	virtual bool doConnectionPreconditionChecking(const QUrl& url);

	// Derived classes must reimplement this function. It should do the
	// heavy lifting required to open the connection with the provided
	// url.
	virtual ConnectionStatus doOpenConnection(const QUrl& connectionUrl) = 0;

	// Derived classes can reimplement this function to perform extra
	// steps when the connection is closed. The base implementation
	// does nothing.
	// Derived classes should also call this function in their destructor,
	// if it reimplements it.
	// This function is called before the connection itself is closed.
	virtual void doCleanUpConnection(void);


	// internal helper function that does the same as closeConnection()
	// but does not call doCloseCOnnection().
	void closeConnectionInternal(void);

	bool connectionInProgress;

	QTimer recoverDeviceTimer;
	QUrl deviceShouldBeConnectedUrl;
    bool deviceRecoveryActive;

	QTimer readTimer;
	int readIndex;

	int dataEmissionChunkSize;
};

#endif // BASEBACKEND_H
