#ifndef FILEBACKEND_H
#define FILEBACKEND_H

#include "basebackend.h"

class QFileSystemWatcher;


class FileBackend : public BaseBackend {
    Q_OBJECT
public:
    FileBackend(QObject *parent = 0);
	virtual ~FileBackend();

	virtual QString getConnectionInfo() const override;

	static const QString protocolScheme;

private slots:
    virtual void onFileChanged();

private:
	virtual bool doConnectionPreconditionChecking(const QUrl& url) override;
	virtual BaseBackend::ConnectionStatus doOpenConnection(const QUrl& connectionUrl) override;
	virtual void doCleanUpConnection(void) override;

	QFileSystemWatcher* watcher;

};

#endif // FILEBACKEND_H
