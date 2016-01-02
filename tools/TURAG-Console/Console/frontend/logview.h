#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QTimer>
#include <QByteArray>
#include <QModelIndex>
#include <tina++/tina.h>
#include <libsimeurobot/fwd.h>
#include <libsimeurobot/appcontext.h>
#include <libsimeurobot/simulation.h>
#include <libsimeurobot/logsourcefilter.h>
#include <libsimeurobot/logsources.h>
#include <libsimeurobot/timeprovider.h>
#include "basefrontend.h"

class TinaInterface;

class RobotLogFrontend : public BaseFrontend {
    Q_OBJECT
    NOT_COPYABLE(RobotLogFrontend);
    NOT_MOVABLE(RobotLogFrontend);

public:
  explicit
  RobotLogFrontend(TinaInterface* tinaInterface, QWidget *parent = 0);

  virtual ~RobotLogFrontend();

  void readSettings();
  void writeSettings();

public slots:
  void onConnected(bool readOnly, QIODevice*) override;
  void onDisconnected(bool reconnecting) override;
  void clear() override;
  void writeData(QByteArray data) override;

protected:
  void keyPressEvent(QKeyEvent * e);

private slots:
  void writeLine(QByteArray line);
  void onUpdateLog();
  void activated(QModelIndex index);

signals:
  void activatedMessage(char source, QString message);

private:
  TURAG::SimEurobot::RobotLogView* log_view_;

  TURAG::SimEurobot::AppContext app_context_;
  TURAG::SimEurobot::LogSourcesFilter filter_;
  TURAG::SimEurobot::LogSources sources_;
  TURAG::SimEurobot::TimeProvider time_;

  QTimer refresh_log_timer_;
};

#endif // LOGVIEW_H
