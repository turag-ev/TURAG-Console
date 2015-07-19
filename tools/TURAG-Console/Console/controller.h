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
class QTabWidget;


class Controller : public QStackedWidget
{
    Q_OBJECT

public:
    explicit Controller(QWidget *parent = 0);
    ~Controller();

	QList<BaseFrontend*> getAvailableFrontends(void) const;
    bool isConnected(void) const;
    QString getConnectionInfo();

    void setExternalMenuBar(QMenuBar* menubar) { menuBar_ = menubar; }
    void setExternalFrontendMenu(QMenu* menu) { frontendMenu = menu; }

    
public slots:
    void openNewConnection(void);
    void openConnection(void);
	void openConnection(const QUrl& connectionUrl, bool* success, BaseBackend **openedBackend);
    void closeConnection(void);
    void cancelNewConnection();
    void setFrontend(int newFrontendIndex);
    void setFrontend(int newFrontendIndex, bool calledManually);
    void saveOutput(void);
    void setAutoReconnect(bool on);
    void setAutoSave(bool on) { autoSaveOn = on; }
    void refresh(void);

protected slots:
	void onConnected(bool readOnly);
    void onDisconnected();
    void onToolboxChangedCurrent(int index);

signals:
    void newConnectionDialogStateChanged(bool visible);
	void connected(bool readOnly, QIODevice*);
    void disconnected(bool reconnecting);


protected:
    QList<BaseBackend*> availableBackends;
    QList<BaseFrontend*> availableFrontends;
    QList<ConnectionWidget*> availableConnectionWidgets;

    BaseBackend* currentBackend;
    int currentFrontendIndex;

    QPushButton* cancelButton;

    QMenuBar* menuBar_;
    QMenu* widgetMenu_;
    QMenu* frontendMenu;
    QTabWidget* tabwidget;

private:
    bool autoSaveOn;
};

#endif // CONTROLLER_H
