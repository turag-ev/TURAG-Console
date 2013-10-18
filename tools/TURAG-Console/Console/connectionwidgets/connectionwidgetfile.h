#ifndef CONNECTIONWIDGETFILE_H
#define CONNECTIONWIDGETFILE_H

#include "connectionwidget.h"
#include <QLineEdit>
#include <QPushButton>


class ConnectionWidgetFile : public ConnectionWidget {
    Q_OBJECT

protected:
    QLineEdit* file_edit;
    QPushButton* file_button;

protected slots:
    void onOpenFile(void);

public:
    ConnectionWidgetFile(QWidget *parent = 0);

};

#endif // CONNECTIONWIDGETFILE_H
