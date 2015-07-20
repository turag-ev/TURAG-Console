#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class Controller;
class QSettings;
class QLabel;
class QSignalMapper;
class QLineEdit;
class QPushButton;
class QSignalMapper;
class QFrame;
class ElidedButton;
class QActionGroup;
class QFont;
class QImage;
class QToolBar;
class LoggerWidget;
class QComboBox;
class ConnectionWidget;
class QTabWidget;
class QStackedWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

	void openUrl(QString url);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void about();
    void resetStatusBar();

    void printError(const QString& message);
    void printMessage(const QString& message);

    void onConnected(bool readOnly);
    void onDisconnected(bool reconnecting);

    void handleNewConnectionAction(bool triggered);
	void updateUrl(const QUrl& url, bool* saveConnectionString);

#   ifdef QT_DEBUG
        void dumpAllObjectTrees(void);
#   endif

private:
	void writeSettings();
	void readSettings();


	Controller* controller;
	QList<ConnectionWidget*> availableConnectionWidgets;


    QAction *connect_action;
    QAction *disconnect_action;
    QAction *new_connection_action;
	QAction* refreshAction;

    QActionGroup* frontendOptions;

    // status bar
    QLabel* status;
    QLabel* permanentStatus;
    QLabel* permanentStatusImage;
    QLabel* permanentReadOnlyImage;
    QPalette default_palette;
    QTimer* status_bar_timer;
    QImage* imgTick;
    QImage* imgCross;
    QImage* imgLock;

    QToolBar* toolbar;
	QToolBar* frontendToolbar;
	QComboBox* addressBar;

    LoggerWidget* logger;

	QTabWidget* tabwidget;
	QPushButton* cancelButton;
	QStackedWidget* centralStackWidget;
};

#endif // MAINWINDOW_H
