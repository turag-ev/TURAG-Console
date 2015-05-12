#ifndef FILEBACKEND_H
#define FILEBACKEND_H

#include "basebackend.h"

class QFileSystemWatcher;

class FileBackend : public BaseBackend {
    Q_OBJECT

public:
    FileBackend(QObject *parent = 0);
	virtual ~FileBackend() {
		closeConnection();
	}

    static const QString protocolScheme;
    virtual QString getConnectionInfo();

public slots:
    virtual bool openConnection(QString connectionString);
    virtual void closeConnection(void);

protected:
    QFileSystemWatcher* watcher;


protected slots:
    virtual void onFileChanged();


};

#endif // FILEBACKEND_H
