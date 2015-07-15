#ifndef CONNECTIONWIDGETFILE_H
#define CONNECTIONWIDGETFILE_H

#include "connectionwidget.h"
#include <QLineEdit>
#include <QPushButton>


class ConnectionWidgetFile : public ConnectionWidget {
    Q_OBJECT
public:
	ConnectionWidgetFile(QWidget *parent = 0);


private slots:
	void onOpenFile(void);

private:
    QLineEdit* file_edit;
    QPushButton* file_button;

};

#endif // CONNECTIONWIDGETFILE_H
