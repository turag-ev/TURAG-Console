#include <QFont>
#include <QTextCodec>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QSettings>
#include <QCloseEvent>
#include <QLabel>
#include <QStatusBar>
#include <QSignalMapper>
#include <QTimer>
#include <QMessageBox>
#include <QLocale>
#include <QProcess>
#include <QImage>
#include <QTreeWidget>
#include <QDialog>
#include <QHBoxLayout>
#include <QDebug>
#include "mainwindow.h"
#include "controller.h"
#include <libs/checkactionext.h>
#include <libs/loggerwidget.h>
#include "connectionwidgets/connectionwidgetfile.h"
#include "connectionwidgets/connectionwidgetserial.h"
#include <libs/iconmanager.h>
#include <libs/log.h>
#include <qt/sidebar/sidebar.h>
#include <QApplication>
#include <frontend/basefrontend.h>

#define _TO_STRING(x) #x
#define TO_STRING(x) _TO_STRING(x)

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent)
{
    // central class that connects backend with frontend and controls
    // information flow
    controller = new Controller(this);
    connect(Log::get(), SIGNAL(fatalMsgAvailable(QString)), this, SLOT(printError(QString)));
    connect(Log::get(), SIGNAL(criticalMsgAvailable(QString)), this, SLOT(printError(QString)));
    connect(Log::get(), SIGNAL(warningMsgAvailable(QString)), this, SLOT(printError(QString)));
    connect(Log::get(), SIGNAL(infoMsgAvailable(QString)), this, SLOT(printMessage(QString)));
	connect(controller, SIGNAL(connected(bool,QIODevice*)), this, SLOT(onConnected(bool)));
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
    new_window->setIcon(IconManager::get("window-new"));
	connect(new_window, &QAction::triggered, [=]() {
		QProcess::startDetached(QCoreApplication::applicationFilePath());
	});

    QAction *save_action = new QAction("&Speichern...", this);
    save_action->setShortcuts(QKeySequence::Save);
    save_action->setStatusTip("Ausgabe speichern");
    save_action->setIcon(IconManager::get("document-save"));
    connect(save_action, SIGNAL(triggered()), controller, SLOT(saveOutput()));

    QAction* save_auto_action = new CheckActionExt("Automatisches Speichern", "Automatisches Speichern", false, this);
    connect(save_auto_action, SIGNAL(triggered(bool)), controller, SLOT(setAutoSave(bool)));

    QAction *exit_action = new QAction("&Beenden", this);
    exit_action->setShortcuts(QKeySequence::Quit);
    exit_action->setStatusTip("Programm verlassen");
    //exit_action->setIcon(IconManager::get("application-exit")));
	connect(exit_action, &QAction::triggered, [this]() {
		controller->closeConnection();
		qApp->closeAllWindows();
	});

    QMenu *file_menu = menuBar()->addMenu("&Datei");
    file_menu->addAction(new_window);
    file_menu->addSeparator();
    file_menu->addAction(save_action);
    file_menu->addAction(save_auto_action);
    file_menu->addSeparator();
    file_menu->addAction(exit_action);

    // Verbindung
    connect_action = new QAction("&Verbinden", this);
    connect_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
    connect_action->setStatusTip("Letzte Verbindung wiederaufbauen");
    connect_action->setIcon(IconManager::get("call-start"));
    connect(connect_action, SIGNAL(triggered()), controller, SLOT(openConnection()));

    disconnect_action = new QAction("&Trennen", this);
    disconnect_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_X));
    disconnect_action->setStatusTip("Aktive Verbindung trennen");
    disconnect_action->setIcon(IconManager::get("call-stop"));
    connect(disconnect_action, SIGNAL(triggered()), controller, SLOT(closeConnection()));

    QAction* auto_reconnect_action = new CheckActionExt("Verbindung offen halten", "Verbindung offen halten", true, this);
    auto_reconnect_action->setStatusTip("Versucht automatisch, verlorene Verbindungen wiederaufzubauen");
    connect(auto_reconnect_action, SIGNAL(triggered(bool)), controller, SLOT(setAutoReconnect(bool)));

	new_connection_action = new QAction("&Verbindungseinstellungen ändern", this);
	new_connection_action->setStatusTip("Eine neue Verbindung aufbauen oder Verbindungseinstellungen ändern");
    new_connection_action->setShortcuts( QList<QKeySequence>{Qt::Key_F2, QKeySequence::Open});
	new_connection_action->setIcon(IconManager::get("preferences-system-network"));
    new_connection_action->setCheckable(true);
    connect(new_connection_action, SIGNAL(triggered(bool)), this, SLOT(handleNewConnectionAction(bool)));
    connect(controller, SIGNAL(newConnectionDialogStateChanged(bool)), new_connection_action, SLOT(setChecked(bool)));

    QMenu *connection_menu = menuBar()->addMenu("&Verbindung");
    connection_menu->addAction(new_connection_action);
    connection_menu->addSeparator();
    connection_menu->addAction(connect_action);
    connection_menu->addAction(disconnect_action);
    connection_menu->addSeparator();
    connection_menu->addAction(auto_reconnect_action);
    connection_menu->addSeparator();

    // Ansicht
    QAction* show_statusbar = new CheckActionExt("Statusleiste anzeigen", "Statusleiste anzeigen", true, this);
    show_statusbar->setStatusTip("Statusleiste anzeigen");
	connect(show_statusbar, &QAction::triggered, [show_statusbar, this]() {
		if (show_statusbar->isChecked()) {
			statusBar()->show();
		} else {
			statusBar()->hide();
		}
	});

//    QAction* show_menubar = new CheckActionExt("Menüleiste anzeigen", "Menüleiste anzeigen", true, this);
//    show_menubar->setStatusTip("Menüleiste anzeigen");
//    show_menubar->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_M));
//    connect(show_menubar, SIGNAL(triggered(bool)), this, SLOT(onShowMenubar(bool)));

	QAction* show_toolbar = new CheckActionExt("Toolbar anzeigen", "Toolbar anzeigen", true, this);
	show_toolbar->setStatusTip("Toolbar anzeigen");
	show_toolbar->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
	connect(show_toolbar, &QAction::triggered, [show_toolbar, this]() {
		if (show_toolbar->isChecked()) {
			toolbar->show();
		} else {
			toolbar->hide();
		}
	});

	QAction* show_frontendToolbar = new CheckActionExt("Frontend-Toolbar anzeigen", "Frontend-Toolbar anzeigen", true, this);
	show_frontendToolbar->setStatusTip("Frontend-Toolbar anzeigen");
	show_frontendToolbar->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T));
	connect(show_frontendToolbar, &QAction::triggered, [show_frontendToolbar, this]() {
		if (show_frontendToolbar->isChecked()) {
			frontendToolbar->show();
		} else {
			frontendToolbar->hide();
		}
	});

	QAction* show_logger = new QAction("Logmeldungen...", this);
//    show_logger->setIcon(IconManager::get("utilities-log-viewer"));
    show_logger->setCheckable(true);
    show_logger->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));

    frontendOptions = new QActionGroup(this);
    QSignalMapper* frontendMapper = new QSignalMapper(this);

    int i = 0;
	for (BaseFrontend* iter : controller->getAvailableFrontends()) {
		QAction* frontendAction = new QAction(iter->objectName(), this);
        frontendAction->setCheckable(true);
        if (i==0) frontendAction->setChecked(true);
		frontendAction->setIcon(iter->getIcon());
        frontendAction->setActionGroup(frontendOptions);
        connect(frontendAction, SIGNAL(triggered()), frontendMapper, SLOT(map()));
        frontendMapper->setMapping(frontendAction, i);
        ++i;
        if (i<=9) {
            frontendAction->setShortcut(QKeySequence(QString("Ctrl+%1").arg(i)));
        }
    }
    connect(frontendMapper, SIGNAL(mapped(int)), controller, SLOT(setFrontend(int)));

	refreshAction = new QAction("Refresh", this);
    refreshAction->setShortcut(QKeySequence(Qt::Key_F5));
    refreshAction->setIcon(IconManager::get("view-refresh"));
	refreshAction->setStatusTip("Daten aus lokalem Puffer neu laden");
	refreshAction->setEnabled(false);
    connect(refreshAction, SIGNAL(triggered()), controller, SLOT(refresh()));

    QMenu* view_menu = menuBar()->addMenu("&Ansicht");
//    view_menu->addAction(show_menubar);
    view_menu->addAction(show_statusbar);
    view_menu->addAction(show_toolbar);
	view_menu->addAction(show_frontendToolbar);
    view_menu->addAction(show_logger);
    view_menu->addSeparator()->setText("Verfügbare Frontends");
    view_menu->addActions(frontendOptions->actions());
    view_menu->addSeparator();
    view_menu->addAction(refreshAction);

	// frontend menu
    //QMenu* frontend_menu = menuBar()->addMenu("frontend");
    //frontend_menu->hide();

    // Hilfe
    QMenu* help_menu = menuBar()->addMenu("&Hilfe");

#   ifdef QT_DEBUG
        QMenu* debug_menu = menuBar()->addMenu("&Debug");
        QAction* objecttree_action = new QAction("Print Objecttree", this);
        objecttree_action->setStatusTip("QT-Objecttree anzeigen");
        connect(objecttree_action, SIGNAL(triggered()), this, SLOT(dumpAllObjectTrees()));
        debug_menu->addAction(objecttree_action);
#   endif


    QAction* about_action = new QAction("&Über", this);
    about_action->setStatusTip("Informationen über TURAG Console");
	about_action->setIcon(IconManager::get("dialog-information"));
    connect(about_action, SIGNAL(triggered()), this, SLOT(about()));
    help_menu->addAction(about_action);

    QAction* about_qt_action = new QAction("Über &Qt", this);
    about_qt_action->setStatusTip("Zeigt Information über die Qt Bibliothek");
    connect(about_qt_action, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    help_menu->addAction(about_qt_action);


    // context menu
    setContextMenuPolicy(Qt::ActionsContextMenu);

    // toolbar
    toolbar = addToolBar("toolbar");
    //toolbar->setIconSize(QSize(20, 20));
    toolbar->setFloatable(false);

	toolbar->addAction(new_window);
	toolbar->addAction(save_action);
	toolbar->addSeparator();
	toolbar->addAction(new_connection_action);
	toolbar->addAction(connect_action);
	toolbar->addAction(disconnect_action);
	toolbar->addSeparator();
	toolbar->addAction(refreshAction);

//    for (QAction* action : menuBar()->actions()) {
//        addAction(action);
//        if (action->menu()) {
//            bool iconsAdded = false;
//            for (QAction*  innerAction : action->menu()->actions()) {
//                if (!innerAction->icon().isNull()) {
//                    toolbar->addAction(innerAction);
//                    iconsAdded = true;
//                }
//            }
//            if (iconsAdded) {
//                toolbar->addSeparator();
//            }
//        }
//	}

	// frontend toolbar
	frontendToolbar = addToolBar("Frontends");
	addToolBar(Qt::LeftToolBarArea, frontendToolbar);
	frontendToolbar->setIconSize(QSize(64, 64));
	frontendToolbar->setFloatable(false);
	frontendToolbar->setMovable(false);
//	frontendToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	frontendToolbar->addActions(frontendOptions->actions());
	QPalette palette = frontendToolbar->palette();
	QColor clr(Qt::darkGray);
	palette.setColor(QPalette::Active, QPalette::Window, clr);
	palette.setColor(QPalette::Inactive, QPalette::Window, clr);
	frontendToolbar->setPalette(palette);

    controller->setExternalMenuBar(menuBar());
    //controller->setExternalFrontendMenu(frontend_menu);

    controller->setAutoSave(save_auto_action->isChecked());
    controller->setAutoReconnect(auto_reconnect_action->isChecked());

	statusBar()->setVisible(show_statusbar->isChecked());
	toolbar->setVisible(show_toolbar->isChecked());
	frontendToolbar->setVisible(show_frontendToolbar->isChecked());

    setWindowTitle("TURAG-Console");
	setWindowIcon(QIcon(":/images/turag-55.png"));

    onDisconnected(false);
    connect_action->setEnabled(false);

    logger = new LoggerWidget;
    connect(show_logger, SIGNAL(triggered(bool)), logger, SLOT(setVisible(bool)));

    QWidget* central_widget = new QWidget;
    QHBoxLayout* mainLayout = new QHBoxLayout;
	mainLayout->setContentsMargins(2, 2, 2, 2);
	mainLayout->addWidget(controller);
    mainLayout->addWidget(logger);
    logger->hide();
    central_widget->setLayout(mainLayout);
    setCentralWidget(central_widget);


	readSettings();
    controller->openNewConnection();
}


void MainWindow::about() {
    QMessageBox::about(this, QString::fromUtf8("Über TURAG-Console"),
                     QString::fromUtf8("<b>TURAG-Console v" TO_STRING(PACKAGE_VERSION) "</b><br />"
                                       "compiled on " __DATE__ " " __TIME__
                                   #ifdef __GNUC__
                                       "<br />gcc " __VERSION__
                                   #endif
                                       "<br /><br />"
                                       "Entwickelt von/für die <a href=\"http://www.turag.de\">TURAG e.V.</a>"
                                       "<br />"
                                       "insbesondere von:<br/><br />"
                                       "&nbsp; Richard Liebscher<br />"
                                       "&nbsp; Martin Oemus<br />"
									   "&nbsp; Pascal Below<br />"
									   "&nbsp; Kevin Seidel<br />"
									   "&nbsp; Florian Völker<br /><br />"
									   "Copyright © 2013 - 2015 TURAG e.V.<br /><br />"
                                       "Based in part on the work of the <a href=\"http://qwt.sf.net\">Qwt project</href>."
                                       ));
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
	refreshAction->setEnabled(true);
    frontendOptions->setEnabled(true);

    status->setText(controller->getConnectionInfo());
    setWindowTitle(controller->getConnectionInfo());

    permanentStatusImage->setPixmap(QPixmap::fromImage(*imgTick));

    if (readOnly) {
        permanentStatus->setText("Verbunden (read-only)");
        permanentReadOnlyImage->setPixmap(QPixmap::fromImage(*imgLock));
    } else {
        permanentStatus->setText("Verbunden");
        permanentReadOnlyImage->setPixmap(QPixmap());
    }
}

void MainWindow::handleNewConnectionAction(bool triggered) {
    if (triggered) {
        controller->openNewConnection();
    } else {
        controller->cancelNewConnection();
    }
}

void MainWindow::onDisconnected(bool reconnecting) {
    if (!reconnecting) {
        connect_action->setEnabled(true);
        disconnect_action->setEnabled(false);
		refreshAction->setEnabled(false);
    }
    status->setText("");
    permanentStatus->setText("Getrennt");

    permanentStatusImage->setPixmap(QPixmap::fromImage(*imgCross));
    permanentReadOnlyImage->setPixmap(QPixmap());
}

#   ifdef QT_DEBUG
void MainWindow::dumpAllObjectTrees(void) {
    class TreeDialog : public QDialog {

    public:
        explicit TreeDialog(QWidget *parent = 0) :
            QDialog(parent)
        {
            tree = new QTreeWidget;
            tree->setColumnCount(3);
            tree->setHeaderLabels(QStringList{QString("Class"), QString("Name"), QString("Geometry")});

            QHBoxLayout* layout = new QHBoxLayout;
            layout->addWidget(tree);
            layout->setMargin(0);
            setLayout(layout);
        }

        void showObjectTree(QObject* base) {
            tree->clear();
            dumpRecursive(base->children());
        }

    private:
        QTreeWidget *tree;

        void dumpRecursive( const QObjectList& list, QTreeWidgetItem *parent = nullptr ) {
            for (QObject* obj : list) {
                if (obj == tree) {
                    continue;
                }

                QString flags;
                if ( obj->isWidgetType() ) {
                    QWidget *w = (QWidget *) obj;
                    if ( w->isVisible() ) {
                        flags.sprintf( "<%d,%d,%d,%d>", w->x(),
                                       w->y(), w->width(),
                                       w->height() );
                    } else {
                        flags = "invisible";
                    }
                }
                QTreeWidgetItem *child;
                child = parent ? new QTreeWidgetItem( parent )
                               : new QTreeWidgetItem( tree );
                child->setText( 0, obj->metaObject()->className() );
                child->setText( 1, obj->objectName() );
                child->setText( 2, flags );
                dumpRecursive( obj->children(), child );
            }
        }
    };


    TreeDialog* treedialog = new TreeDialog(this);
    treedialog->open();
    treedialog->showObjectTree(this);

}
#   endif


void MainWindow::readSettings() {
  QSettings settings;

  setWindowState(static_cast<Qt::WindowStates>(settings.value("state", 0).toInt()));
  resize(settings.value("size", QSize(1100, 850)).toSize());

  int currentFrontend = settings.value("currentFrontend", 0).toInt();
  frontendOptions->actions().at(currentFrontend)->setChecked(true);
  controller->setFrontend(currentFrontend);
}

void MainWindow::writeSettings() {
  QSettings settings;
  settings.setValue("size", size());
  settings.setValue("state", static_cast<int>(windowState()));

  QList<QAction*> actions = frontendOptions->actions();
  int currentFrontend = actions.indexOf(frontendOptions->checkedAction());
  if (currentFrontend != -1) {
      settings.setValue("currentFrontend", currentFrontend);
  }
}

MainWindow::~MainWindow()
{
	IconManager::clear();
}

void MainWindow::closeEvent(QCloseEvent *event) {
  writeSettings();
  event->accept();
}

void MainWindow::openUrl(QString url_string) {
	if (url_string.isEmpty()) {
		return;
	}

	qDebug() << "got url" << url_string;

	QUrl url(QUrl::fromUserInput(url_string));

	if (!url.isValid()) {
		qDebug() << "Url not parsable, trying file";
		url.setScheme("file");
		QFile file(url_string);
		if (file.exists()) {
			url.setPath(file.fileName());
		}
	}

	bool success = false;
	controller->openConnection(url, &success, nullptr);
	if (!success) {
		printError("Couldn't open specified connection");
		controller->openNewConnection();
	}
}

