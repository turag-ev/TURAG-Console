#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class ConnectionWidget;
class Controller;
class ElidedButton;
class LoggerWidget;
class PopupContainerWidget;
class QActionGroup;
class QComboBox;
class QFont;
class QFrame;
class QImage;
class QLabel;
class QLineEdit;
class QPushButton;
class QSettings;
class QSignalMapper;
class QStackedLayout;
class QTabWidget;
class QToolBar;

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

	void updateUrl(const QUrl& url);

#   ifdef QT_DEBUG
        void dumpAllObjectTrees(void);
#   endif

	void handleNewConnectionAction(bool checked);
	void showConnectionWidgetOverlay(void);
	void hideConnectionWidgetOverlay(void);

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
	ElidedButton* frontendButton;

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
	QColor addressBarColorBase;
	QColor addressBarColorText;

    LoggerWidget* logger;

	QTabWidget* connectionTabWidget;
//	QPushButton* cancelButton;
	PopupContainerWidget* popupContainer;
};

#endif // MAINWINDOW_H
