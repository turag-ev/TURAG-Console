#include <QFont>
#include <QTextCodec>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QSettings>
#include <QCloseEvent>
#include <QLabel>
#include <QStatusBar>
#include <QSignalMapper>
#include <QTimer>
#include <QMessageBox>
#include <QLocale>
#include <QProcess>
#include <QCoreApplication>
#include <QImage>
#include "mainwindow.h"
#include "controller.h"
#include "connectionwidgets/connectionwidgetfile.h"
#include "connectionwidgets/connectionwidgetserial.h"

#if QT_VERSION < 0x050000
# include <QtGui/QApplication>
#else
# include <QApplication>
#endif



MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent)
{
    // central class that connects backend with frontend and controls
    // information flow
    controller = new Controller(this);
    connect(controller, SIGNAL(errorOccured(QString)), this, SLOT(printError(QString)));
    connect(controller, SIGNAL(infoMessage(QString)), this, SLOT(printMessage(QString)));
    connect(controller, SIGNAL(connected(bool,bool,QIODevice*)), this, SLOT(onConnected(bool)));
    connect(controller, SIGNAL(disconnected(bool)), this, SLOT(onDisconnected(bool)));


    // adjust statusbar
    status = new QLabel();
    permanentStatus = new QLabel();
    permanentStatusImage = new QLabel();
    permanentReadOnlyImage = new QLabel();
    permanentReadOnlyImage->setToolTip("Read-Only Zugriff");


    statusBar()->addWidget(status);
    statusBar()->addPermanentWidget(permanentReadOnlyImage);
    statusBar()->addPermanentWidget(permanentStatusImage);
    statusBar()->addPermanentWidget(permanentStatus);
    statusBar()->setSizeGripEnabled(true);

    default_palette = statusBar()->palette();
    status_bar_timer = new QTimer(this);
    status_bar_timer->setSingleShot(true);
    connect(status_bar_timer, SIGNAL(timeout()), this, SLOT(resetStatusBar()));

    imgTick = new QImage(":/images/ok.png");
    imgCross = new QImage(":/images/nok.png");
    imgLock = new QImage(":/images/lock.png");


    // create menu structure

    // Datei
    QAction *new_window = new QAction("&Neues Fenster", this);
    new_window->setShortcut(QKeySequence::New);
    new_window->setShortcutContext(Qt::ApplicationShortcut);
    new_window->setStatusTip("Neues Konsolen-Fenster öffnen");
    connect(new_window, SIGNAL(triggered()), this, SLOT(onNewWindow()));

    QAction *save_action = new QAction("&Speichern...", this);
    save_action->setShortcuts(QKeySequence::Save);
    save_action->setStatusTip("Ausgabe speicheren");
    connect(save_action, SIGNAL(triggered()), controller, SLOT(saveOutput()));

    QAction *exit_action = new QAction("&Beenden", this);
    exit_action->setShortcuts(QKeySequence::Quit);
    exit_action->setStatusTip("Programm verlassen");
    connect(exit_action, SIGNAL(triggered()), this, SLOT(close()));

    QMenu *file_menu = menuBar()->addMenu("&Datei");
    file_menu->addAction(new_window);
    file_menu->addSeparator();
    file_menu->addAction(save_action);
    file_menu->addSeparator();
    file_menu->addAction(exit_action);

    // Verbindung
    connect_action = new QAction("&Verbinden", this);
    connect_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
    connect_action->setStatusTip("Letzte Verbindung wiederaufbauen");
    connect(connect_action, SIGNAL(triggered()), controller, SLOT(openConnection()));

    disconnect_action = new QAction("&Trennen", this);
    disconnect_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_X));
    disconnect_action->setStatusTip("Aktive Verbindung trennen");
    connect(disconnect_action, SIGNAL(triggered()), controller, SLOT(closeConnection()));

    auto_reconnect_action = new QAction("Verbindung offen halten", this);
    auto_reconnect_action->setStatusTip("Versucht automatisch, verlorene Verbindungen wiederaufzubauen");
    auto_reconnect_action->setCheckable(true);
    connect(auto_reconnect_action, SIGNAL(triggered(bool)), controller, SLOT(setAutoReconnect(bool)));

    QAction *new_connection_action = new QAction("&Neue Sitzung...", this);
    new_connection_action->setStatusTip("Neue Sitzung öffnen");
    new_connection_action->setShortcut(QKeySequence::Open);
    connect(new_connection_action, SIGNAL(triggered()), controller, SLOT(openNewConnection()));

    QMenu *connection_menu = menuBar()->addMenu("&Verbindung");
    connection_menu->addAction(new_connection_action);
    connection_menu->addSeparator();
    connection_menu->addAction(connect_action);
    connection_menu->addAction(disconnect_action);
    connection_menu->addSeparator();
    connection_menu->addAction(auto_reconnect_action);
    connection_menu->addSeparator();

    // Ansicht
    show_statusbar = new QAction("Statusleiste anzeigen", this);
    show_statusbar->setCheckable(true);
    show_statusbar->setStatusTip("Statusleiste anzeigen");
    connect(show_statusbar, SIGNAL(triggered(bool)), this, SLOT(onShowStatusbar(bool)));

    show_menubar = new QAction("Menüleiste anzeigen", this);
    show_menubar->setCheckable(true);
    show_menubar->setStatusTip("Menüleiste anzeigen");
    show_menubar->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_M));
    connect(show_menubar, SIGNAL(triggered(bool)), this, SLOT(onShowMenubar(bool)));

    frontendOptions = new QActionGroup(this);
    QSignalMapper* frontendMapper = new QSignalMapper(this);

    QList<QString> frontendList = controller->getAvailableFrontends();

    int i = 0;
    for (auto iter : frontendList) {
        QAction* frontendAction = new QAction(iter, this);
        frontendAction->setCheckable(true);
        if (i==0) frontendAction->setChecked(true);
        frontendAction->setActionGroup(frontendOptions);
        connect(frontendAction, SIGNAL(triggered()), frontendMapper, SLOT(map()));
        frontendMapper->setMapping(frontendAction, i);
        ++i;
        if (i<=9) {
            frontendAction->setShortcut(QKeySequence(QString("Ctrl+%1").arg(i)));
        }
    }
    connect(frontendMapper, SIGNAL(mapped(int)), controller, SLOT(setFrontend(int)));

    QMenu* view_menu = menuBar()->addMenu("&Ansicht");
    view_menu->addAction(show_menubar);
    view_menu->addAction(show_statusbar);
    view_menu->addSeparator()->setText("Verfügbare Frontends");
    view_menu->addActions(frontendOptions->actions());


    // Hilfe
    QAction* about_action = new QAction("&Über", this);
    about_action->setStatusTip("Informationen über TURAG Console");
    connect(about_action, SIGNAL(triggered()), this, SLOT(about()));

    QAction* about_qt_action = new QAction("Über &Qt", this);
    about_qt_action->setStatusTip("Zeigt Information über die Qt Bibliothek");
    connect(about_qt_action, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    QMenu* help_menu = menuBar()->addMenu("&Hilfe");
    help_menu->addAction(about_action);
    help_menu->addAction(about_qt_action);

    // context menu
    setContextMenuPolicy(Qt::ActionsContextMenu);

    for (auto iter : menuBar()->actions()) {
        addAction(iter);
    }

    controller->setExternalContextActions(actions());
    controller->setExternalMenuBar(menuBar());
    controller->setExternalConnectionMenu(connection_menu);

    setWindowTitle("TURAG-Console");
	setWindowIcon(QIcon(":/images/turag-55.png"));

    onDisconnected(false);
    connect_action->setEnabled(false);
    setCentralWidget(controller);
    readSettings();

    controller->openNewConnection();
}


void MainWindow::about() {
  QMessageBox::about(this, QString::fromUtf8("Über TURAG-Console"),
                     QString::fromUtf8("<b>TURAG-Console v2.0</b><br /><br />"
                                       "Entwickelt von/für die <a href=\"http://www.turag.de\">TURAG e.V.</a>"
                                       "<br />"
                                       "insbesondere von:<br/><br />"
                                       "&nbsp; Richard Liebscher<br />"
                                       "&nbsp; Martin Oemus<br /><br />"
                                       "Copyright © 2013 TURAG e.V.<br /><br />"
                                       "Based in part on the work of the <a href=\"http://qwt.sf.net\">Qwt project</href>."
                                       ));
}

void MainWindow::onNewWindow() {
    QProcess::startDetached(QCoreApplication::applicationFilePath());
}

void MainWindow::close() {
  controller->closeConnection();
  qApp->closeAllWindows();
}

void MainWindow::onShowStatusbar(bool show) {
    if (show) {
        statusBar()->show();
        show_statusbar->setChecked(true);
    } else {
        statusBar()->hide();
        show_statusbar->setChecked(false);
    }
}

void MainWindow::onShowMenubar(bool show) {
    if (show) {
        menuBar()->show();
        show_menubar->setChecked(true);
    } else {
        menuBar()->hide();
        show_menubar->setChecked(false);
    }
}

void MainWindow::printError(const QString &message) {
  QPalette palette;
  palette.setColor(QPalette::WindowText, QColor(255, 0, 0));
  statusBar()->setPalette(palette);

  statusBar()->showMessage(message, 0);
  status_bar_timer->start(5000);
}


void MainWindow::printMessage(const QString &message) {
    statusBar()->setPalette(default_palette);
    statusBar()->showMessage(message, 5000);
}


void MainWindow::resetStatusBar() {
  statusBar()->setPalette(default_palette);
  statusBar()->clearMessage();
}


void MainWindow::onConnected(bool readOnly) {
    connect_action->setEnabled(false);
    disconnect_action->setEnabled(true);
    frontendOptions->setEnabled(true);

    status->setText(controller->getConnectionInfo());

    permanentStatusImage->setPixmap(QPixmap::fromImage(*imgTick));

    if (readOnly) {
        permanentStatus->setText("Verbunden (read-only)");
        permanentReadOnlyImage->setPixmap(QPixmap::fromImage(*imgLock));
    } else {
        permanentStatus->setText("Verbunden");
        permanentReadOnlyImage->setPixmap(QPixmap());
    }
}

void MainWindow::onDisconnected(bool reconnecting) {
    if (!reconnecting) {
        connect_action->setEnabled(true);
        disconnect_action->setEnabled(false);
    }
    frontendOptions->setEnabled(false);

    status->setText("");
    permanentStatus->setText("Getrennt");

    permanentStatusImage->setPixmap(QPixmap::fromImage(*imgCross));
    permanentReadOnlyImage->setPixmap(QPixmap());
}


void MainWindow::readSettings() {
  QSettings settings;

  setWindowState(static_cast<Qt::WindowStates>(settings.value("state", 0).toInt()));
  resize(settings.value("size", QSize(1100, 850)).toSize());
  onShowStatusbar(settings.value("showStatusBar", true).toBool());
  onShowMenubar(settings.value("showMenuBar", true).toBool());
  auto_reconnect_action->setChecked(settings.value("AutoReconnect", true).toBool());
  controller->setAutoReconnect(settings.value("AutoReconnect", true).toBool());

  int currentFrontend = settings.value("currentFrontend", 0).toInt();
  frontendOptions->actions().at(currentFrontend)->setChecked(true);
  controller->setFrontend(currentFrontend);
}

void MainWindow::writeSettings() {
  QSettings settings;
  settings.setValue("size", size());
  settings.setValue("state", static_cast<int>(windowState()));
  settings.setValue("showStatusBar", show_statusbar->isChecked());
  settings.setValue("showMenuBar", show_menubar->isChecked());
  settings.setValue("AutoReconnect", auto_reconnect_action->isChecked());

  QList<QAction*> actions = frontendOptions->actions();
  int currentFrontend = actions.indexOf(frontendOptions->checkedAction());
  if (currentFrontend != -1) {
      settings.setValue("currentFrontend", currentFrontend);
  }
}

MainWindow::~MainWindow() { }

void MainWindow::closeEvent(QCloseEvent *event) {
  writeSettings();
  event->accept();
}

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  QLocale curLocale(QLocale("de_DE"));
  QLocale::setDefault(curLocale);
  setlocale(LC_ALL, "de");

#if QT_VERSION < 0x050000
  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

  QFont::insertSubstitution("Consolas", "Ubuntu Mono");
  QFont::insertSubstitution("Consolas", "Monospace");
  QFont::insertSubstitution("Consolas", "Courier New");

//  QFont font;
//  font.setFamily(font.defaultFamily());
//  font.setPointSize(8);
//  a.setFont(font);

  QCoreApplication::setOrganizationName("TURAG");
  QCoreApplication::setOrganizationDomain("turag.de");
  QCoreApplication::setApplicationName("Console");

  MainWindow w;
  w.show();

  return a.exec();
}
