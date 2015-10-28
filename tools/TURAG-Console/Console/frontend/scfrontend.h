#ifndef SCFRONTEND_H
#define SCFRONTEND_H

#include <libsimeurobot/fwd.h>
#include <libsimeurobot/appcontext.h>
#include <libsimeurobot/simulation.h>
#include <libsimeurobot/robotcontext.h>
#include <libsimeurobot/robotbase.h>
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

private slots:
	void seek();

private:
	TinaInterface tina_interface_;

	TURAG::SimEurobot::RobotLogView* log_view_;
	TURAG::SimEurobot::RobotModelsView* robot_model_;
	TURAG::SimEurobot::RobotsTreeModel* robot_model_tree_model_;
	TURAG::SimEurobot::Scene* scene_;
	QMainWindow* dock_area_;

	TURAG::SimEurobot::AppContext appcontext_;
	TURAG::SimEurobot::Simulation simcontext_;
	TURAG::SimEurobot::RobotBase robot_;

	QTimer refresh_log_timer_;

	void advance();
};

#endif // SCFRONTEND_H
