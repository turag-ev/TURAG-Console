#ifndef SCFRONTEND_H
#define SCFRONTEND_H

#include <libsimeurobot/fwd.h>
#include <libsimeurobot/appcontext.h>
#include <libsimeurobot/simcontext.h>
#include <libsimeurobot/robotcontext.h>
#include <libsimeurobot/scenemanager.h>
#include <QTimer>

#include "basefrontend.h"
#include "util/tinainterface.h"

class TinaInterface;
class QTimer;
class QMainWindow;

class SCFrontend : public BaseFrontend
{
	Q_OBJECT
public:
	SCFrontend(QWidget* parent = 0);
	~SCFrontend();

	void readSettings();
	void writeSettings();

	// BaseFrontend interface
public slots:
	void writeData(QByteArray data);
	void clear();
	void onConnected(bool readOnly, QIODevice*);
	void onDisconnected(bool reconnecting);

	void writeLine(QByteArray line);

private:
	TinaInterface tina_interface_;

	TURAG::SimEurobot::RobotLogView* log_view_;
	TURAG::SimEurobot::RobotModelsView* robot_model_;
	TURAG::SimEurobot::RobotsTreeModel* robot_model_tree_model_;
	TURAG::SimEurobot::Scene* scene_;
	QMainWindow* dock_area_;
/*
	TURAG::SimEurobot::AppContext app_context_;
	TURAG::SimEurobot::SimContext sim_context_;*/
	TURAG::SimEurobot::SceneManager sim_manager_;
	TURAG::SimEurobot::RobotModule& robot_;
	TURAG::SimEurobot::RobotContext& robot_context_;

	QTimer refresh_log_timer_;
};

#endif // SCFRONTEND_H
