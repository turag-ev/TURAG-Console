#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QFont>
#include <QStackedWidget>

class BaseFrontend;
class BaseBackend;
class ConnectionWidget;
class QPushButton;
class QAction;
class QTimer;


class Controller : public QStackedWidget
{
    Q_OBJECT

protected:
    QList<BaseBackend*> availableBackends;
    QList<BaseFrontend*> availableFrontends;
    QList<ConnectionWidget*> availableConnectionWidgets;

    BaseBackend* currentBackend;
    int currentFrontendIndex;

    QPushButton* cancelButton;

    bool autoReconnect;
    bool connectionShouldBeOpen;
    QTimer* reconnectTimer;


public:
    explicit Controller(QWidget *parent = 0);

    QList<QString> getAvailableFrontends(void) const;
    bool isConnected(void) const;
    QString getConnectionInfo();

    void setExternalContextActions(QList<QAction*> actions);

    
public slots:
    void openNewConnection(void);
    void openConnection(void);
    void closeConnection(void);
    void setFrontend(int newFrontendIndex);
    void saveOutput(void);
    void setAutoReconnect(bool on);

protected slots:
    void openConnection(QString connectionString, bool* success);
    void onCancelNewConnection();
    void onConnected(bool readOnly, bool isSequential);
    void onDisconnected();
    void onErrorOccured(QString msg);
    void onInfoMessage(QString msg);
    void onReconnectTimeout();

signals:
    void connected(bool readOnly, bool isSequential);
    void disconnected(bool reconnecting);
    void errorOccured(QString msg);
    void infoMessage(QString msg);

    
};

#endif // CONTROLLER_H
