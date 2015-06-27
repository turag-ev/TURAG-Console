#include "scfrontend.h"

#include <QTextStream>
#include <tina++/algorithm.h>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QDockWidget>
#include <QSettings>
#include <QTimer>
#include <tina/debug/defines.h>
#include <libsimeurobot/ui/robotlogview.h>
#include <libsimeurobot/ui/robots.h>
#include <libsimeurobot/ui/treemodel/treemodel.h>
#include <libsimeurobot/ui/treemodel/treeitem.h>
#include <libsimeurobot/ui/treemodel/robotitem.h>
#include <libsimeurobot/ui/scene.h>
#include <libsimeurobot/scenes/eurobot2015.h>
#include <libsimeurobot/parser.h>

using namespace TURAG::SimEurobot;

SCFrontend::SCFrontend(QWidget *parent)
	: BaseFrontend(QStringLiteral("Meldungen"), parent),
	  tina_interface_(nullptr),
//	  sim_context_(app_context_),
//	  robot_context_(sim_context_),
	  sim_manager_(true),
	  robot_(sim_manager_.getRobotModule(ROBOT_1A)),
	  robot_context_(robot_.getRobotContext())
{
	// Scene
	scene_ = new Scene(sim_manager_);

	// dock area
	dock_area_ = new QMainWindow;
	dock_area_->setCentralWidget(scene_);
	dock_area_->setDockOptions(QMainWindow::AnimatedDocks
						   //  | QMainWindow::AllowNestedDocks
							   | QMainWindow::AllowTabbedDocks);
	QDockWidget* dock_widget;

	// RobotModelsView
	dock_widget = new QDockWidget("Roboter", dock_area_);
	dock_widget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	dock_widget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

	robot_model_tree_model_ = new RobotsTreeModel(this);
	TreeItem* root_item = robot_model_tree_model_->getRoot();
	auto* robot_model_item = new RobotModelItem(robot_);
	root_item->appendChild(robot_model_item);
	robot_model_ = new RobotModelsView(dock_widget);
	robot_model_->setModel(robot_model_tree_model_);
	dock_widget->setWidget(robot_model_);
	dock_area_->addDockWidget(Qt::RightDockWidgetArea, dock_widget);

	connect(&sim_manager_, SIGNAL(onPlayingAreaChanged(const PlayingArea*)),
			robot_model_, SLOT(onPlayingAreaChanged(const PlayingArea*)));

	// RobotLogView
	dock_widget = new QDockWidget("Nachrichten", dock_area_);
	dock_widget->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
	dock_widget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
	log_view_ = new RobotLogView(sim_manager_.getRobotModule(ROBOT_1A).getRobotContext());
	dock_widget->setWidget(log_view_);
	dock_area_->addDockWidget(Qt::BottomDockWidgetArea, dock_widget);

	// build gui
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(dock_area_);
    layout->setMargin(0);
    setLayout(layout);

	sim_manager_.pause();
	sim_manager_.init(scene_);
	sim_manager_.setPlayingArea(new EuroBot2015());
	sim_manager_.getRobotModule(ROBOT_1A).init(scene_, log_view_, robot_model_item);


	// connect to input source TinaInterface
    connect(&tina_interface_, SIGNAL(tinaPackageReady(QByteArray)),
			this, SLOT(writeLine(QByteArray)));

	connect(&refresh_log_timer_, SIGNAL(timeout()),
			log_view_, SLOT(updateLog()));

    readSettings();
}


SCFrontend::~SCFrontend()
{
	writeSettings();
}

void SCFrontend::readSettings()
{
	QSettings settings;
	settings.beginGroup("SCFrontend");
	robot_context_.readSettings(&settings);
}

void SCFrontend::writeSettings()
{
	QSettings settings;
	settings.beginGroup("SCFrontend");
	robot_context_.writeSettings(&settings);
}

void SCFrontend::onConnected(bool readOnly, QIODevice* dev)
{
	UNUSED(dev);

	log_view_->setScrolling(true);
    log_view_->setReadOnly(readOnly);
	refresh_log_timer_.start(500);
}

void SCFrontend::onDisconnected(bool reconnecting)
{
    UNUSED(reconnecting);

	refresh_log_timer_.stop();
}

void SCFrontend::writeLine(QByteArray line)
{
	DebugMessage message = parseDebugMessagePayload(line);

	if (robot_context_.handle(message))
		return;

	log_view_->insertRow(message);
}

void SCFrontend::writeData(QByteArray data)
{
	tina_interface_.dataInput(data);
}

void SCFrontend::clear()
{
	log_view_->clear();
	tina_interface_.clear();
}
