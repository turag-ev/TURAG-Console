#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QTimer>
#include <QByteArray>
#include <tina++/tina.h>
#include <libsimeurobot/fwd.h>
#include <libsimeurobot/timeprovider.h>
#include "basefrontend.h"

class TinaInterface;

class LogFileTimeProvider : public TURAG::SimEurobot::TimeProvider {
public:
	unsigned getTime() const;

	void setTime(unsigned time) { last_log_time_ = time; }
	void parseGameTimeMessage(const QString& message);

private:
	unsigned last_log_time_ = 0;
};

class RobotLogFrontend : public BaseFrontend {
    Q_OBJECT
    NOT_COPYABLE(RobotLogFrontend);
    NOT_MOVABLE(RobotLogFrontend);

public:
  explicit
  RobotLogFrontend(TinaInterface* interface, QWidget *parent = 0);

  virtual ~RobotLogFrontend();

  void setLogSource(char source, const QString& name);

  /*void readSettings();
  void writeSettings();*/

public slots:
  void onConnected(bool readOnly, QIODevice*) override;
  void onDisconnected(bool reconnecting) override;
  void clear() override;
  void writeData(QByteArray data) override;

protected:
  void keyPressEvent ( QKeyEvent * e );

private slots:
  void writeLine(QByteArray line);
  void onSendTimeout();
  void onUpdateLog();

signals:
  void activatedGraph(int index);

private:
  TURAG::SimEurobot::RobotLogView* log_view_;

  LogFileTimeProvider time_provider_;
  QTimer refresh_log_timer_;

  QTimer sendTimer;
  QByteArray timedSendString;
};

#endif // LOGVIEW_H
