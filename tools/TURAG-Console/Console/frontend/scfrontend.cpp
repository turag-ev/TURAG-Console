#include "scfrontend.h"

#include <QTextStream>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QDockWidget>
#include <QSettings>
#include <QTimer>
#include <QDebug>

#include <tina++/algorithm.h>
#include <tina++/time.h>
#include <libs/iconmanager.h>
#include <tina/debug/defines.h>
#include <libsimeurobot/ui/robotlogview.h>
#include <libsimeurobot/ui/robots.h>
#include <libsimeurobot/ui/treemodel/treemodel.h>
#include <libsimeurobot/ui/treemodel/logitem.h>
#include <libsimeurobot/ui/treemodel/layeritem.h>
#include <libsimeurobot/ui/scene.h>
#include <libsimeurobot/scenes/eurobot2015.h>
#include <libsimeurobot/parser.h>
#include <libsimeurobot/robots/unknownrobot.h>
#include <libsimeurobot/ui/logcontext.h>
#include <libsimeurobot/vis/visualization.h>

using namespace TURAG::SimEurobot;
using namespace TURAG;

class ConsoleRobotVis : public RobotVisualization
{
public:
    explicit ConsoleRobotVis(SCFrontend& scf)
        : scf_(scf)
    { }

    void onRobotChanged(std::uint8_t id, std::int16_t playingarea_id) override
    {
        auto pa = PlayingArea::fromId(playingarea_id);
        if (pa)
            scf_.simcontext_.setPlayingArea(std::move(pa));
    }

    void onPrepared() override
    {
		//scf_.simcontext_.setRobotsPrepared(true);
    }

    void onGameStarted() override
    {
		//scf_.simcontext_.beginGame();
    }

    void onExtraTimeStarted() override
    {

    }

    void onGameStopped() override
    {
		//scf_.simcontext_.endGame();
    }

private:
    SCFrontend& scf_;
};

SCFrontend::SCFrontend(QWidget *parent)
    : BaseFrontend(QStringLiteral("SystemControl Debug"),
                   IconManager::get("eurobot"), parent),
	  tina_interface_(nullptr),
	  appcontext_(),
	  simcontext_(appcontext_),
      robot_(simcontext_, std::unique_ptr<ConsoleRobotVis>(new ConsoleRobotVis(*this)))
{
	// Scene
	scene_ = new Scene(simcontext_);

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
    root_item->appendChild(new LayerGroup(robot_.getVisualization().getLayers()));
    root_item->appendChild(new LogSourcesGroup(robot_.getLogSources(), filter_));

	robot_model_ = new RobotModelsView(dock_widget);
	robot_model_->setModel(robot_model_tree_model_);
	dock_widget->setWidget(robot_model_);
	dock_area_->addDockWidget(Qt::RightDockWidgetArea, dock_widget);

	// RobotLogView
	dock_widget = new QDockWidget("Nachrichten", dock_area_);
	dock_widget->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
	dock_widget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
	log_view_ = new RobotLogView(LogContext(filter_, robot_.getLogSources(), robot_.getTimeProvider(), robot_.getSimulation().getAppContext()));
	dock_widget->setWidget(log_view_);
	dock_area_->addDockWidget(Qt::BottomDockWidgetArea, dock_widget);

	// build gui
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(dock_area_);
    layout->setMargin(0);
    setLayout(layout);

    // backend
    simcontext_.setPlayingArea(PlayingArea::fromId(PlayingArea::getLastestId()));
	scene_->addRobot(&robot_);

	// connect to input source TinaInterface
    connect(&tina_interface_, SIGNAL(tinaPackageReady(QByteArray)),
			this, SLOT(writeLine(QByteArray)));

	connect(&refresh_log_timer_, &QTimer::timeout, log_view_, &RobotLogView::updateLog);

    readSettings();

	connect(&simcontext_, SIGNAL(seeked()), this, SLOT(seek()));
}


SCFrontend::~SCFrontend()
{
	writeSettings();
}

void SCFrontend::readSettings()
{
	QSettings settings;
	settings.beginGroup("SCFrontend");
	filter_.readSettings(&settings);
}

void SCFrontend::writeSettings()
{
	QSettings settings;
	settings.beginGroup("SCFrontend");
	filter_.writeSettings(&settings);
}

void SCFrontend::onConnected(bool readOnly, QIODevice* dev)
{
	UNUSED(dev);

	log_view_->setScrolling(true);
    log_view_->setReadOnly(readOnly);
	refresh_log_timer_.start(500);
	simcontext_.resumeSimulation();
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

	if (!simcontext_.isPlaybackRunning()
		&& !simcontext_.isSimulationPaused())
	{
		SystemTime robot_time = robot_.getTimeProvider().getTime() + robot_.getRobotTimeOffset();
		if (robot_time > simcontext_.getSimulationPresentTime())
			simcontext_.seekTo(robot_time);
	}
}

void SCFrontend::seek()
{
    robot_.update();
}

void SCFrontend::writeData(QByteArray data_)
{
	tina_interface_.dataInput(data_);
}

void SCFrontend::clear()
{
	log_view_->clear();
	tina_interface_.clear();
	simcontext_.endGame();
}
