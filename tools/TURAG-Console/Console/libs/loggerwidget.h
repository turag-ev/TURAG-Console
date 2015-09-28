#ifndef LOGGERWIDGET_H
#define LOGGERWIDGET_H

#include <QWidget>

class QTableView;
class QComboBox;
class QPushButton;
class QCloseEvent;


class LoggerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LoggerWidget(QWidget *parent = 0);

signals:

public slots:

protected:
	void closeEvent(QCloseEvent *event);

private:
    QTableView* log;
    QPushButton* clear;
    QPushButton* save;


};

#endif // LOGGERWIDGET_H
