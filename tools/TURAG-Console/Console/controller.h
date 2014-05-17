#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QStackedWidget>
#include <QByteArray>

class BaseFrontend;
class BaseBackend;
class ConnectionWidget;
class QPushButton;
class QAction;
class QMenuBar;
class QMenu;
class QToolBox;


class Controller : public QStackedWidget
{
    Q_OBJECT

public:
    explicit Controller(QWidget *parent = 0);
    ~Controller();

    QList<QString> getAvailableFrontends(void) const;
    bool isConnected(void) const;
    QString getConnectionInfo();

    void setExternalContextActions(QList<QAction*> actions);
    void setExternalMenuBar(QMenuBar* menubar) {menuBar_ = menubar; }
    void setExternalConnectionMenu(QMenu* menu) {connectionMenu = menu; }

    
public slots:
    void openNewConnection(void);
    void openConnection(void);
    void openConnection(QString connectionString, bool* success, BaseBackend **openedBackend);
    void closeConnection(void);
    void setFrontend(int newFrontendIndex);
    void setFrontend(int newFrontendIndex, bool calledManually);
    void saveOutput(void);
    void setAutoReconnect(bool on);
    void setAutoSave(bool on) { autoSaveOn = on; }

protected slots:
    void onCancelNewConnection();
    void onConnected(bool readOnly, bool isBuffered);
    void onDisconnected();
    void onErrorOccured(QString msg);
    void onInfoMessage(QString msg);
    void onToolboxChangedCurrent(int index);
    void saveDataToBuffer(QByteArray data);

signals:
    void connected(bool readOnly, bool isBuffered, QIODevice*);
    void disconnected(bool reconnecting);
    void errorOccured(QString msg);
    void infoMessage(QString msg);


protected:
    QList<BaseBackend*> availableBackends;
    QList<BaseFrontend*> availableFrontends;
    QList<ConnectionWidget*> availableConnectionWidgets;

    BaseBackend* currentBackend;
    int currentFrontendIndex;

    QPushButton* cancelButton;

    QMenuBar* menuBar_;
    QMenu* widgetMenu_;
    QMenu* connectionMenu;
    QToolBox* toolbox;

private:
    bool saveBufferToFile(QString fileName);

    bool autoSaveOn;
    QByteArray* buffer;
    
};

#endif // CONTROLLER_H
