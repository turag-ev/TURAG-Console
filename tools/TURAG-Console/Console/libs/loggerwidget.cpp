#include "loggerwidget.h"
#include "log.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QSettings>
#include <QCloseEvent>


LoggerWidget::LoggerWidget(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout* vlayout = new QVBoxLayout;

    log = new QTableView;
	log->setModel(Log::model());
	QHeaderView* hheader = log->horizontalHeader();
	hheader->setSectionResizeMode(0, QHeaderView::Fixed);
	hheader->resizeSection(0, 80);
	hheader->setSectionResizeMode(1, QHeaderView::Fixed);
	hheader->resizeSection(1, 80);
	hheader->setSectionResizeMode(2, QHeaderView::Stretch);
	hheader->setSectionResizeMode(3, QHeaderView::Stretch);

	connect(log, &QTableView::activated, [this] {
		log->resizeRowToContents(log->currentIndex().row());
	});

    vlayout->addWidget(log);

    QHBoxLayout* hlayout = new QHBoxLayout;
    save = new QPushButton("Speichern");
    hlayout->addWidget(save);
    clear = new QPushButton("Löschen");
    hlayout->addWidget(clear);

    vlayout->addLayout(hlayout);
    setLayout(vlayout);

	QSettings settings;
	resize(settings.value("loggerwidget-size", QSize(800, 600)).toSize());
	move(settings.value("loggerwidget-pos", QPoint(0, 0)).toPoint());
}


void LoggerWidget::closeEvent(QCloseEvent *event) {
	QSettings settings;
	settings.setValue("loggerwidget-size", size());
	settings.setValue("loggerwidget-pos", pos());

	event->accept();
}
