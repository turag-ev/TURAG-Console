#include "scfrontend.h"

#include <QTextStream>
#include <tina++/algorithm.h>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QDockWidget>
#include <QSettings>
#include <QTimer>
#include <libs/iconmanager.h>
#include <tina/debug/defines.h>
#include <libsimeurobot/ui/robotlogview.h>
#include <libsimeurobot/ui/robots.h>
#include <libsimeurobot/ui/treemodel/treemodel.h>
#include <libsimeurobot/ui/treemodel/treeitem.h>
#include <libsimeurobot/ui/scene.h>
#include <libsimeurobot/scenes/eurobot2015.h>
#include <libsimeurobot/parser.h>
#include <libsimeurobot/robots/unknownrobot.h>

using namespace TURAG::SimEurobot;

namespace {

class PlaybackRobotItem : public TreeItem {
public:
	PlaybackRobotItem()
	{

	}

	QVariant data(int column, int role) const override;
};

QVariant PlaybackRobotItem::data(int column, int role) const
{
	return QVariant();
}

}

SCFrontend::SCFrontend(QWidget *parent)
	: BaseFrontend(QStringLiteral("SystemControl Debug"), IconManager::get("eurobot"), parent),
	  tina_interface_(nullptr),
	  appcontext_(),
	  simcontext_(appcontext_),
	  robot_(simcontext_)
{
	// Scene
	scene_ = new Scene(simcontext_);
	robot_.init(new UnknownRobot);

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
	root_item->appendChild(new PlaybackRobotItem());
	robot_model_ = new RobotModelsView(dock_widget);
	robot_model_->setModel(robot_model_tree_model_);
	dock_widget->setWidget(robot_model_);
	dock_area_->addDockWidget(Qt::RightDockWidgetArea, dock_widget);

	// RobotLogView
	dock_widget = new QDockWidget("Nachrichten", dock_area_);
	dock_widget->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
	dock_widget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
	log_view_ = new RobotLogView(robot_.getRobotContext());
	dock_widget->setWidget(log_view_);
	dock_area_->addDockWidget(Qt::BottomDockWidgetArea, dock_widget);

	// build gui
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(dock_area_);
    layout->setMargin(0);
    setLayout(layout);

	simcontext_.setPlayingArea(std::make_shared<EuroBot2015>());
	scene_->addRobot(&robot_);

	// connect to input source TinaInterface
    connect(&tina_interface_, SIGNAL(tinaPackageReady(QByteArray)),
			this, SLOT(writeLine(QByteArray)));

	connect(&refresh_log_timer_, SIGNAL(timeout()),
			log_view_, SLOT(updateLog()));

    readSettings();

	connect(&simcontext_, SIGNAL(seeked()), this, SLOT(seek()));
	simcontext_.registerAdvanceCallback(std::bind(&SCFrontend::seek, this));
}


SCFrontend::~SCFrontend()
{
	writeSettings();
}

void SCFrontend::readSettings()
{
	QSettings settings;
	settings.beginGroup("SCFrontend");
	robot_.getRobotContext().readSettings(&settings);
}

void SCFrontend::writeSettings()
{
	QSettings settings;
	settings.beginGroup("SCFrontend");
	robot_.getRobotContext().writeSettings(&settings);
}

void SCFrontend::onConnected(bool readOnly, QIODevice* dev)
{
	UNUSED(dev);

	log_view_->setScrolling(true);
    log_view_->setReadOnly(readOnly);
	refresh_log_timer_.start(500);
	simcontext_.resumeSimulation();
	simcontext_.setRobotsPrepared();
	simcontext_.beginGame();
}

void SCFrontend::onDisconnected(bool reconnecting)
{
    UNUSED(reconnecting);

	refresh_log_timer_.stop();
	simcontext_.pauseSimulation();
}

void SCFrontend::writeLine(QByteArray line)
{
	DebugMessage dm;
	if (!parseDebugMessagePayload(line, dm))
		return;

	robot_.log(dm);
	log_view_->insertRow(dm);
}

void SCFrontend::seek()
{
	robot_.goToTime(simcontext_.getSimulationCurrentTime());
}

void SCFrontend::writeData(QByteArray data)
{
	tina_interface_.dataInput(data);
}

void SCFrontend::clear()
{
	log_view_->clear();
	tina_interface_.clear();
	simcontext_.endGame();
}
