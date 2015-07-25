#include <QFont>
#include <QTextCodec>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QToolButton>
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
#include <QComboBox>
#include <QApplication>
#include <QScrollArea>
#include <QtNetwork>
#include <QtWidgets>

#include <libs/checkactionext.h>
#include <libs/loggerwidget.h>
#include <libs/iconmanager.h>
#include <libs/log.h>
#include <libs/elidedbutton.h>
//#include <qt/sidebar/sidebar.h>
#include <tina/helper/macros.h>

#include "mainwindow.h"
#include "controller.h"
#include <frontend/basefrontend.h>

#include "connectionwidgets/connectionwidgetfile.h"
#include "connectionwidgets/connectionwidgetserial.h"
#include "connectionwidgets/connectionwidgettcp.h"
#include "connectionwidgets/connectionwidgetwebdav.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent)
{
	setWindowTitle("TURAG-Console");
	setWindowIcon(IconManager::get("turag-55"));


	// add all available connectionWidgets to list without a parent
	availableConnectionWidgets.append(new ConnectionWidgetSerial);
	availableConnectionWidgets.append(new ConnectionWidgetFile);
	availableConnectionWidgets.append(new ConnectionWidgetTcp);
	availableConnectionWidgets.append(new ConnectionWidgetWebDAV);


	// central class that connects backend with frontend and controls
    // information flow
    controller = new Controller(this);
    connect(Log::get(), SIGNAL(fatalMsgAvailable(QString)), this, SLOT(printError(QString)));
    connect(Log::get(), SIGNAL(criticalMsgAvailable(QString)), this, SLOT(printError(QString)));
    connect(Log::get(), SIGNAL(warningMsgAvailable(QString)), this, SLOT(printError(QString)));
    connect(Log::get(), SIGNAL(infoMsgAvailable(QString)), this, SLOT(printMessage(QString)));
	connect(controller, SIGNAL(connected(bool,QIODevice*)), this, SLOT(onConnected(bool)));
    connect(controller, SIGNAL(disconnected(bool)), this, SLOT(onDisconnected(bool)));


    // create menu structure
	// ---------------------------------------

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
	exit_action->setIcon(IconManager::get("application-exit"));
	connect(exit_action, &QAction::triggered, [this]() {
		controller->closeConnection();
		qApp->closeAllWindows();
	});

    // Verbindung
	connect_action = new QAction("&Verbinden (Strg+Y)", this);
    connect_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
    connect_action->setStatusTip("Letzte Verbindung wiederaufbauen");
    connect_action->setIcon(IconManager::get("call-start"));
	connect(connect_action, &QAction::triggered, [this]() {
		controller->openConnection(addressBar->currentText(), nullptr, nullptr);
	});

	disconnect_action = new QAction("&Trennen (Strg+X)", this);
    disconnect_action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_X));
    disconnect_action->setStatusTip("Aktive Verbindung trennen");
    disconnect_action->setIcon(IconManager::get("call-stop"));
    connect(disconnect_action, SIGNAL(triggered()), controller, SLOT(closeConnection()));

    QAction* auto_reconnect_action = new CheckActionExt("Verbindung offen halten", "Verbindung offen halten", true, this);
    auto_reconnect_action->setStatusTip("Versucht automatisch, verlorene Verbindungen wiederaufzubauen");
    connect(auto_reconnect_action, SIGNAL(triggered(bool)), controller, SLOT(setAutoReconnect(bool)));

	new_connection_action = new QAction("&Verbindungseinstellungen ändern (F2)", this);
	new_connection_action->setStatusTip("Eine neue Verbindung aufbauen oder Verbindungseinstellungen ändern");
    new_connection_action->setShortcuts( QList<QKeySequence>{Qt::Key_F2, QKeySequence::Open});
	new_connection_action->setIcon(IconManager::get("preferences-system-network"));
	new_connection_action->setCheckable(true);
	new_connection_action->setChecked(true);
    connect(new_connection_action, SIGNAL(triggered(bool)), this, SLOT(handleNewConnectionAction(bool)));

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


	QAction* show_logger = new QAction("Logmeldungen...", this);
//    show_logger->setIcon(IconManager::get("utilities-log-viewer"));
    show_logger->setCheckable(true);

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

	refreshAction = new QAction("Daten neu laden (F5)", this);
    refreshAction->setShortcut(QKeySequence(Qt::Key_F5));
    refreshAction->setIcon(IconManager::get("view-refresh"));
	refreshAction->setStatusTip("Daten aus lokalem Puffer neu laden");
	refreshAction->setEnabled(false);
    connect(refreshAction, SIGNAL(triggered()), controller, SLOT(refresh()));


    // Hilfe
	QAction* about_action = new QAction("&Über", this);
	about_action->setStatusTip("Informationen über TURAG Console");
	about_action->setIcon(IconManager::get("dialog-information"));
	connect(about_action, SIGNAL(triggered()), this, SLOT(about()));


#   ifdef QT_DEBUG
        QAction* objecttree_action = new QAction("Print Objecttree", this);
        objecttree_action->setStatusTip("QT-Objecttree anzeigen");
        connect(objecttree_action, SIGNAL(triggered()), this, SLOT(dumpAllObjectTrees()));
#   endif


	// toolbar
	// ---------------------------------------
	toolbar = addToolBar("toolbar");
//	toolbar->setIconSize(QSize(20, 20));
	toolbar->setFloatable(false);
	toolbar->setMovable(false);


	QMenu* main_menu = new QMenu(this);
	main_menu->addAction(new_window);
	main_menu->addAction(save_action);
	main_menu->addAction(about_action);
	main_menu->addSeparator();
	main_menu->addAction(show_statusbar);
	main_menu->addAction(show_logger);
	main_menu->addAction(auto_reconnect_action);
	main_menu->addAction(save_auto_action);
#   ifdef QT_DEBUG
		main_menu->addAction(objecttree_action);
#   endif
	main_menu->addSeparator();
	main_menu->addAction(exit_action);

	QToolButton* hamburger_button = new QToolButton;
	hamburger_button->setIcon(IconManager::get("hamburger"));
	hamburger_button->setMenu(main_menu);
	hamburger_button->setPopupMode(QToolButton::InstantPopup);
//	QAction* hamburger_action = new QAction("Menu", this);
//	hamburger_action->setIcon(IconManager::get("hamburger"));
//	hamburger_action->setMenu(complete_menu);
//	connect(hamburger_action, &QAction::triggered, [this,complete_menu]() {
//		complete_menu->popup(toolbar->mapToGlobal(QPoint(0, toolbar->height())));
//	});

	// address bar
	addressBar = new QComboBox;
	addressBar->setEditable(true);
	addressBar->setMinimumWidth(250);
	addressBar->setMaximumWidth(1000);
	addressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	addressBar->lineEdit()->setPlaceholderText("Insert URL here... (press Ctrl+L to focus)");
	connect(addressBar->lineEdit(), SIGNAL(returnPressed()), connect_action, SLOT(trigger()));
	QAction* focusAddressbarAction = new QAction(this);
	addressBar->addAction(focusAddressbarAction);
	focusAddressbarAction->setShortcut(Qt::CTRL | Qt::Key_L);
	connect(focusAddressbarAction, &QAction::triggered, [this]() {
		addressBar->setFocus();
		addressBar->lineEdit()->selectAll();
	});

	QWidget* spacerWidget = new QWidget;
	spacerWidget->setMinimumWidth(30);

	QMenu* frontendButtonMenu = new QMenu(this);
	frontendButtonMenu->addActions(frontendOptions->actions());
	frontendButton = new ElidedButton;
	frontendButton->setElideMode(Qt::ElideRight);
	frontendButton->setText(frontendOptions->checkedAction()->text());
	frontendButton->setIcon(frontendOptions->checkedAction()->icon());
	frontendButton->setMenu(frontendButtonMenu);
	frontendButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	frontendButton->setMinimumWidth(150);
	frontendButton->setMaximumWidth(350);
	frontendButton->setIconSize(toolbar->iconSize());
	frontendButton->setToolTip("Verfügbare Frontends");
	connect(frontendOptions, &QActionGroup::triggered, [this]() {
		frontendButton->setText(frontendOptions->checkedAction()->text());
		frontendButton->setIcon(frontendOptions->checkedAction()->icon());
	});


	// assemble toolbar
	toolbar->addWidget(hamburger_button);
	toolbar->addSeparator();
	toolbar->addAction(new_connection_action);
	toolbar->addWidget(addressBar);
	toolbar->addAction(connect_action);
	toolbar->addAction(disconnect_action);
	toolbar->addSeparator();
	toolbar->addAction(refreshAction);
	toolbar->addWidget(spacerWidget);
	toolbar->addSeparator();
	toolbar->addWidget(frontendButton);


	// frontend toolbar
//	frontendToolbar = new QToolBar("Frontends");
//	addToolBar(Qt::LeftToolBarArea, frontendToolbar);
//	frontendToolbar->setIconSize(QSize(64, 64));
//	frontendToolbar->setFloatable(false);
//	frontendToolbar->setMovable(false);
//	frontendToolbar->addActions(frontendOptions->actions());
//	QPalette palette = frontendToolbar->palette();
//	QColor clr(Qt::darkGray);
//	palette.setColor(QPalette::Active, QPalette::Window, clr);
//	palette.setColor(QPalette::Inactive, QPalette::Window, clr);
//	frontendToolbar->setPalette(palette);


	// adjust statusbar
	// ---------------------------------------
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
	statusBar()->setVisible(show_statusbar->isChecked());

	default_palette = statusBar()->palette();
	status_bar_timer = new QTimer(this);
	status_bar_timer->setSingleShot(true);
	connect(status_bar_timer, SIGNAL(timeout()), this, SLOT(resetStatusBar()));

	imgTick = new QImage(":/images/ok.png");
	imgCross = new QImage(":/images/nok.png");
	imgLock = new QImage(":/images/lock.png");


	// build welcome screen
	// -------------------------------------------------
	QVBoxLayout* layout = new QVBoxLayout();
	QWidget* welcome_screen = new QWidget();

	connectionTabWidget = new QTabWidget;

	for (ConnectionWidget* iter : availableConnectionWidgets) {
		QScrollArea* scrollarea = new QScrollArea;
		scrollarea->setWidgetResizable(true);
		scrollarea->setWidget(iter);
		scrollarea->setFrameShape(QFrame::NoFrame);

		connectionTabWidget->addTab(scrollarea, iter->objectName());
		connect(iter, SIGNAL(connectionChanged(QUrl,bool*,BaseBackend**)), controller, SLOT(openConnection(QUrl, bool*,BaseBackend**)));
		connect(iter, SIGNAL(connectionChanged(QUrl,bool*,BaseBackend**)), this, SLOT(updateUrl(QUrl,bool*)));
	}
	layout->addWidget(connectionTabWidget);
	connect(connectionTabWidget, &QTabWidget::currentChanged, [this]() {
		QSettings settings;
		settings.beginGroup("Controller");
		settings.setValue("currentIndex", connectionTabWidget->currentIndex());
	});

	if (connectionTabWidget->count() > 0) {
		QSettings settings;
		settings.beginGroup("Controller");
		connectionTabWidget->setCurrentIndex(settings.value("currentIndex", 0).toInt());
	}

	cancelButton = new QPushButton("Abbrechen");
	layout->addSpacing(5);
	layout->addWidget(cancelButton, 0, Qt::AlignLeft);
	connect(cancelButton, &QPushButton::clicked, [this]() {
		new_connection_action->setChecked(false);
		centralStackWidget->setCurrentIndex(1);
		frontendButton->setEnabled(true);
		refreshAction->setEnabled(true);
	});

	welcome_screen->setLayout(layout);



	// build central widget
	// ---------------------------------------
	logger = new LoggerWidget;
    connect(show_logger, SIGNAL(triggered(bool)), logger, SLOT(setVisible(bool)));

	centralStackWidget = new QStackedWidget;
    QWidget* central_widget = new QWidget;
    QHBoxLayout* mainLayout = new QHBoxLayout;
	mainLayout->setContentsMargins(2, 2, 2, 2);
	mainLayout->addWidget(controller);
    mainLayout->addWidget(logger);
    logger->hide();
    central_widget->setLayout(mainLayout);
	centralStackWidget->addWidget(welcome_screen);
	centralStackWidget->addWidget(central_widget);

	setCentralWidget(centralStackWidget);
	frontendButton->setEnabled(false);
	refreshAction->setEnabled(false);




//	QTabWidget* tabWidget = new QTabWidget;
//	QAction* closeTabAction = new QAction(this);
//	closeTabAction->setShortcut(Qt::CTRL | Qt::Key_W);
//	connect(closeTabAction, &QAction::triggered, [tabWidget]() {
//		if (tabWidget->count() > 1) {
//			tabWidget->removeTab(tabWidget->currentIndex());
//		}
//	});
//	tabWidget->addAction(closeTabAction);
//	tabWidget->setMovable(true);

//	connect(tabWidget->tabBar(), &QTabBar::tabCloseRequested, [tabWidget](int index) {
//		if (tabWidget->count() > 1) {
//			tabWidget->removeTab(index);
//		}
//	});
//	tabWidget->tabBar()->setAutoHide(true);
//	QToolButton *tb = new QToolButton();
//	tb->setIcon(IconManager::get("tab-new"));
//	tb->setAutoRaise(true);
//	tb->setIconSize(QSize(16,16));
//	QAction* newTabAction = new QAction(this);
//	connect(newTabAction, &QAction::triggered, [tabWidget]() {
//		int newIndex = tabWidget->currentIndex() + 1;
//		tabWidget->insertTab(tabWidget->currentIndex() + 1, new QWidget, "Neuer Tab");
//		tabWidget->setCurrentIndex(newIndex);
//	});
//	newTabAction->setShortcut(Qt::CTRL | Qt::Key_T);
//	tabWidget->addAction(newTabAction);
//	connect(tb, SIGNAL(clicked()), newTabAction, SLOT(trigger()));

//	tabWidget->setCornerWidget(tb, Qt::TopRightCorner);
//	tabWidget->setTabsClosable(true);
//	tabWidget->insertTab(tabWidget->count() - 1, new QWidget, "Neuer Tab");
//	tabWidget->setCurrentIndex(tabWidget->count() - 2);

//	setCentralWidget(tabWidget);



	controller->setAutoSave(save_auto_action->isChecked());
	controller->setAutoReconnect(auto_reconnect_action->isChecked());

	onDisconnected(false);
	readSettings();
}


void MainWindow::about() {
	QString aboutText(QString::fromUtf8("<b>TURAG-Console v" TURAG_STRINGIFY(PACKAGE_VERSION) "</b><br />"
										"compiled on " __DATE__ " " __TIME__
									#ifdef __GNUC__
										"<br />with gcc " __VERSION__
									#endif
										"<br />built with Qt v" QT_VERSION_STR
										"<br />running on Qt v%1"
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
										).arg(qVersion()));

	QMessageBox aboutBox(QMessageBox::Information, QString("About %1").arg(qApp->applicationName()), aboutText, QMessageBox::Ok, this);
	QPushButton* aboutQtButton = aboutBox.addButton("About Qt...", QMessageBox::HelpRole);
	connect(aboutQtButton, SIGNAL(pressed()), qApp, SLOT(aboutQt()));
	aboutBox.exec();
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
//	connect_action->setVisible(false);
    disconnect_action->setEnabled(true);
//	disconnect_action->setVisible(true);
	refreshAction->setEnabled(true);
//    frontendOptions->setEnabled(true);
	new_connection_action->setChecked(false);
	centralStackWidget->setCurrentIndex(1);
	frontendButton->setEnabled(true);
	refreshAction->setEnabled(true);

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

void MainWindow::handleNewConnectionAction(bool ) {
	if (new_connection_action->isChecked()) {
		centralStackWidget->setCurrentIndex(0);
		frontendButton->setEnabled(false);
		refreshAction->setEnabled(false);
	} else {
		centralStackWidget->setCurrentIndex(1);
		frontendButton->setEnabled(true);
		refreshAction->setEnabled(true);
	}
}

void MainWindow::updateUrl(const QUrl& url, bool* ) {
	addressBar->lineEdit()->setText(url.toDisplayString());
}

void MainWindow::onDisconnected(bool reconnecting) {
    if (!reconnecting) {
        connect_action->setEnabled(true);
//		connect_action->setVisible(true);
		disconnect_action->setEnabled(false);
//		disconnect_action->setVisible(false);
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
  frontendButton->setText(frontendOptions->checkedAction()->text());
  frontendButton->setIcon(frontendOptions->checkedAction()->icon());
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
		centralStackWidget->setCurrentIndex(0);
	}
}

