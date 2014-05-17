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

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void openConnection(QString connection_string);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void onNewWindow();
    void close();

    void onShowStatusbar(bool show);
    void onShowMenubar(bool show);
    void onShowToolbar(bool show);

    void about();
    void resetStatusBar();

    void printError(const QString& message);
    void printMessage(const QString& message);

    void onConnected(bool readOnly);
    void onDisconnected(bool reconnecting);

#   ifdef QT_DEBUG
        void dumpAllObjectTrees(void);
#   endif

private:
    Controller* controller;

    QAction *connect_action;
    QAction *disconnect_action;

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

    void writeSettings();
    void readSettings();
};

#endif // MAINWINDOW_H
