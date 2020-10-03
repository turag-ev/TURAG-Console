#ifndef TURAG_CONSOLE_CONSOLE_CONNECTIONWIDGETS_CONNECTIONWIDGETFILE_H
#define TURAG_CONSOLE_CONSOLE_CONNECTIONWIDGETS_CONNECTIONWIDGETFILE_H

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

#endif // TURAG_CONSOLE_CONSOLE_CONNECTIONWIDGETS_CONNECTIONWIDGETFILE_H
