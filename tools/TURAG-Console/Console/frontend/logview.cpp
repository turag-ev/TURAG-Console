#include "logview.h"

#include <QTextStream>
#include <tina++/algorithm.h>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QDebug>
#include <tina/debug/defines.h>
#include <libsimeurobot/ui/robotlogview.h>
#include <libsimeurobot/parser.h>

#include "util/tinainterface.h"

using namespace TURAG::SimEurobot;

unsigned LogFileTimeProvider::getTime() const
{
	return last_log_time_;
}

void LogFileTimeProvider::parseGameTimeMessage(const QString& message)
{
	unsigned hex_time;
    if (sscanf(message.toLatin1().data(), "%x", &hex_time) == 1) // TODO: Perf
	{
        setTime(hex_time);
    }
}

RobotLogFrontend::RobotLogFrontend(TinaInterface *interface, QWidget *parent) :
	BaseFrontend(QStringLiteral("Meldungen"), parent),
	robot_context_(app_context_, log_sources_, log_filter_, time_provider_)
{
	log_view_ = new RobotLogView(robot_context_);

    // build gui
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(log_view_);
    layout->setMargin(0);
    setLayout(layout);

    // connect to input source TinaInterface
    connect(interface, SIGNAL(tinaPackageReady(QByteArray)), this, SLOT(writeLine(QByteArray)));

	connect(&refresh_log_timer_, SIGNAL(timeout()), this, SLOT(onUpdateLog()));

    connect(&sendTimer, SIGNAL(timeout()), this, SLOT(onSendTimeout()));

    //readSettings();
}

RobotLogFrontend::~RobotLogFrontend() {
	//writeSettings();
}

void RobotLogFrontend::setLogSource(char source, const QString& name)
{
	robot_context_.getLogSources().setLogSource(source, name);
}

void RobotLogFrontend::onSendTimeout()
{
    if (timedSendString.size()) {
        emit dataReady(timedSendString.left(1));
        timedSendString.remove(0,1);
    }
    if (timedSendString.size() == 0) {
        sendTimer.stop();
	}
}

void RobotLogFrontend::onUpdateLog()
{
	log_view_->updateLog();
}

static
const char* charToKey(char c)
{
    static char key[2];
    key[0] = c;
    key[1] = '\0';
    return key;
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

/* TODO    QSettings settings;
    const std::string& filter = filter_->getFilterSource();
    for (char key : filter) {
        if (TURAG::any_of_value(filter, key)) {
            settings.setValue(charToKey(key), false);
        } else {
            settings.remove(charToKey(key));
        }
    }*/
}

/* TODO
void RobotLogFrontend::readSettings() {
    QSettings settings;
    settings.beginGroup(objectName());
	filter_->setFilterSource(settings.value(QStringLiteral("filter"), QString()).toString().toStdString());
}

void RobotLogFrontend::writeSettings() {
    QSettings settings;
    settings.beginGroup(objectName());
	settings.setValue(QStringLiteral("filter"), QString::fromStdString(filter_->getFilterSource()));
}*/

static
bool handleGraphMessage(DebugMessage& input)
{
	if (input.log_source == TURAG_DEBUG_GRAPH_CREATE[0])
	{
		QTextStream stream(&input.message, QIODevice::ReadOnly);
		int index = 0;
		stream >> index;

		input.log_source = ';';
		input.level = ';';
		input.message = QStringLiteral("Graph %1: '%2'")
						.arg(index).arg(stream.readAll().trimmed());
		return true;
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
						.arg(index).arg(stream.readAll().trimmed());
		return true;
	}

	return false;
}

void RobotLogFrontend::writeLine(QByteArray line)
{
	// TODO: Code nach LogModel verscheiben

	DebugMessage message = parseDebugMessagePayload(line);
	switch (message.level) {
	case TURAG_DEBUG_ERROR_PREFIX[0]:
	case TURAG_DEBUG_CRITICAL_PREFIX[0]:
	case TURAG_DEBUG_WARN_PREFIX[0]:
	case TURAG_DEBUG_INFO_PREFIX[0]:
	case TURAG_DEBUG_DEBUG_PREFIX[0]:
	case ';':
		log_view_->insertRow(message);
		break;

	case TURAG_DEBUG_GAMETIME_PREFIX[0]:
		time_provider_.parseGameTimeMessage(message.message);
		break;

	case TURAG_DEBUG_GRAPH_PREFIX[0]:
		if (handleGraphMessage(message)) {
			log_view_->insertRow(message);
		}
		break;

	case TURAG_DEBUG_REPORT_LOG_SOURCE_PREFIX[0]:
		if (std::isprint(message.log_source)) {
			setLogSource(message.log_source, message.message);
		}
		break;
	}
}

void RobotLogFrontend::clear(void)
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
