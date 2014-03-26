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

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onNewWindow();
    void close();

    void onShowStatusbar(bool show);
    void onShowMenubar(bool show);

    void about();
    void resetStatusBar();

    void printError(const QString& message);
    void printMessage(const QString& message);

    void onConnected(bool readOnly);
    void onDisconnected(bool reconnecting);

#   ifdef QT_DEBUG
        void dumpAllObjectTrees(void);
#   endif

protected:
    void closeEvent(QCloseEvent *event);

private:
    Controller* controller;

    QAction *connect_action;
    QAction *disconnect_action;
    QAction* auto_reconnect_action;
    QAction* show_statusbar;
    QAction* show_menubar;

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

    void writeSettings();
    void readSettings();
};

#endif // MAINWINDOW_H
