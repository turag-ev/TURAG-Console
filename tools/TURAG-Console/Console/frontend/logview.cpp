#include "logview.h"

#include <QTextStream>
#include <tina++/algorithm.h>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QDebug>
#include <QSettings>
#include <QTableView>
#include <tina/debug/defines.h>
#include <libsimeurobot/ui/robotlogview.h>
#include <libsimeurobot/ui/robotlogmodel.h>
#include <libsimeurobot/parser.h>
#include <libsimeurobot/ui/logcontext.h>

#include "util/tinainterface.h"

using namespace TURAG::SimEurobot;

RobotLogFrontend::RobotLogFrontend(TinaInterface *tinaInterface, QWidget *parent) :
	BaseFrontend(QStringLiteral("Meldungen"), QIcon(), parent)
{	
	log_view_ = new RobotLogView(LogContext(filter_, sources_, time_, app_context_));
    connect(log_view_->getTable(), SIGNAL(activated(QModelIndex)),
			this, SLOT(activated(QModelIndex)));

    // build gui
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(log_view_);
    layout->setMargin(0);
    setLayout(layout);

    // connect to input source TinaInterface
    connect(tinaInterface, SIGNAL(tinaPackageReady(QByteArray)), this, SLOT(writeLine(QByteArray)));

	connect(&refresh_log_timer_, SIGNAL(timeout()), this, SLOT(onUpdateLog()));

    readSettings();
}

RobotLogFrontend::~RobotLogFrontend()
{
	writeSettings();
}

void RobotLogFrontend::readSettings()
{
	QSettings settings;
	settings.beginGroup("RobotLogFrontend");
	filter_.readSettings(&settings);
}

void RobotLogFrontend::writeSettings()
{
	QSettings settings;
	settings.beginGroup("RobotLogFrontend");
	filter_.writeSettings(&settings);
}

void RobotLogFrontend::onUpdateLog()
{
	log_view_->updateLog();
}

void RobotLogFrontend::activated(QModelIndex index)
{
	using namespace TURAG::SimEurobot;

	if (!index.isValid()) return;

	const QAbstractItemModel* model = index.model();
	QModelIndex message_index = index.sibling(index.row(), RobotLogModel::COLUMN_MESSAGE);
	QModelIndex source_index = index.sibling(index.row(), RobotLogModel::COLUMN_SOURCE);
	emit activatedMessage(
				model->data(source_index).toChar().toLatin1(),
				model->data(message_index).toString());
}

void RobotLogFrontend::onConnected(bool readOnly, QIODevice* dev)
{
	UNUSED(dev);

	log_view_->setScrolling(true);
    log_view_->setReadOnly(readOnly);
	refresh_log_timer_.start(500);
}

void RobotLogFrontend::onDisconnected(bool reconnecting)
{
    UNUSED(reconnecting);

	refresh_log_timer_.stop();
}

static
bool handleGraphMessage(DebugMessage& input)
{
	if (input.level != TURAG_DEBUG_GRAPH_PREFIX[0]) return false;

	if (input.log_source == TURAG_DEBUG_GRAPH_CREATE[0])
	{
		QTextStream stream(&input.message, QIODevice::ReadOnly);
		int index = 0;
		stream >> index;

		input.log_source = ';';
		input.level = ';';
		input.message = QStringLiteral("Graph %1: '%2'")
						.arg(index).arg(stream.readAll().trimmed()).toUtf8();
		return false;
	}
	else if (input.log_source == TURAG_DEBUG_GRAPH_COPY[0])
	{
		QTextStream stream(&input.message, QIODevice::ReadOnly);
		int index = 0, old_index;
		stream >> index;
		stream >> old_index;

		input.log_source = ';';
		input.level = ';';
		input.message = QStringLiteral("Graph %1: '%2'")
						.arg(index).arg(stream.readAll().trimmed()).toUtf8();
		return false;
	}

	return true;
}

void RobotLogFrontend::writeLine(QByteArray line)
{
	DebugMessage message;
	if (!parseDebugMessagePayload(line, message))
		return;

	if (time_.handle(message))
		return;

	if (sources_.handle(message))
		return;

	// Graphen
	if (handleGraphMessage(message))
		return;

	log_view_->insertRow(message);
}

void RobotLogFrontend::clear()
{
    log_view_->clear();
}

// we need to implement this function as part of the base classes interface
// but we don't actually need it.
void RobotLogFrontend::writeData(QByteArray data)
{
	Q_UNUSED(data);
}

void RobotLogFrontend::keyPressEvent ( QKeyEvent * e )
{
	if (e->count() > 0) {
		qDebug() << "PTF keyPressEvent: '" << e->text().toUtf8() << "'";
		emit dataReady(e->text().toUtf8());
	} else {
		BaseFrontend::keyPressEvent(e);
	}
}
