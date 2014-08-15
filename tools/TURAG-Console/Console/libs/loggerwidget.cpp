#include "loggerwidget.h"
#include "log.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableView>


LoggerWidget::LoggerWidget(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout* vlayout = new QVBoxLayout;
    log = new QTableView;
    log->setModel(Log::model());
    vlayout->addWidget(log);

    QHBoxLayout* hlayout = new QHBoxLayout;
    save = new QPushButton("Speichern");
    hlayout->addWidget(save);
    clear = new QPushButton("Löschen");
    hlayout->addWidget(clear);

    vlayout->addLayout(hlayout);
    setLayout(vlayout);

}
