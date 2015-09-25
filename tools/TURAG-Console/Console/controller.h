#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QStackedWidget>
#include <QByteArray>

class BaseFrontend;
class BaseBackend;


class Controller : public QStackedWidget
{
    Q_OBJECT

public:
    explicit Controller(QWidget *parent = 0);
    ~Controller();

	QList<BaseFrontend*> getAvailableFrontends(void) const;
    bool isConnected(void) const;
    QString getConnectionInfo();

    
public slots:
    void openConnection(void);
	void openConnection(const QUrl& connectionUrl, bool* success);
    void closeConnection(void);
    void setFrontend(int newFrontendIndex);
    void setFrontend(int newFrontendIndex, bool calledManually);
    void saveOutput(void);
    void setAutoReconnect(bool on);
    void setAutoSave(bool on) { autoSaveOn = on; }
    void refresh(void);

protected slots:
	void onConnected(bool readOnly);
    void onDisconnected();

signals:
	void connected(bool readOnly, QIODevice*);
    void disconnected(bool reconnecting);


protected:
    QList<BaseBackend*> availableBackends;
    QList<BaseFrontend*> availableFrontends;

    BaseBackend* currentBackend;
    int currentFrontendIndex;


private:
    bool autoSaveOn;
};

#endif // CONTROLLER_H
