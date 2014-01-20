#ifndef FILEBACKEND_H
#define FILEBACKEND_H

#include "basebackend.h"

class QFileSystemWatcher;

class FileBackend : public BaseBackend {
    Q_OBJECT

protected:
    QFileSystemWatcher* watcher;


protected slots:
    virtual void onFileChanged();


public:
    FileBackend(QObject *parent = 0);

    static const QString connectionPrefix;
    static const bool networked;
    virtual QString getConnectionInfo();

public slots:
    virtual bool openConnection(QString connectionString);
    virtual void checkData(void);
    virtual void closeConnection(void);

};

#endif // FILEBACKEND_H
