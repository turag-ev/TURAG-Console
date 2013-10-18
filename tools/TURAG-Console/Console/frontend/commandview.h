#ifndef COMMANDVIEW_H
#define COMMANDVIEW_H

#include <QWidget>
#include <QString>
#include "basefrontend.h"
#include "util/lineinterface.h"
#include <QByteArray>

class QTextEdit;
class QLineEdit;


class CommandView : public BaseFrontend {
  Q_OBJECT
public:
  explicit CommandView(QWidget *parent = 0);

  void flush();
  void setScrollOnOutput(bool on);

signals:

private slots:
  void onChar(QString character);
  void scroll();
  void writeLine(QByteArray data);

public slots:
  virtual void writeData(QByteArray data);
  virtual void clear(void);
  virtual bool saveOutput(void);

private:
  LineInterface* lineInterface;

  QTextEdit* output_;
  QLineEdit* input_;

  QString buffer_;

  bool scroll_on_output_;
};

#endif // COMMANDVIEW_H
