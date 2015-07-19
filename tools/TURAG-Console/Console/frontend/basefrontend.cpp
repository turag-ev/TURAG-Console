#include "basefrontend.h"
#include <QList>
#include <QAction>
#include <QMenu>
#include <QPoint>
#include <QScrollArea>
#include <QFrame>
#include <QHBoxLayout>

BaseFrontend::BaseFrontend(QString name, const QIcon &icon_, QWidget *parent) :
	QWidget(parent), icon(icon_)
{
    setObjectName(name);

	setContextMenuPolicy(Qt::ActionsContextMenu);

	scrollFrame = new QFrame;

	QScrollArea* scrollarea = new QScrollArea;
	scrollarea->setWidgetResizable(true);
	scrollarea->setFrameStyle(0);
	scrollarea->setWidget(scrollFrame);

	QHBoxLayout* layout = new QHBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(scrollarea);
	QWidget::setLayout(layout);
}

void BaseFrontend::setLayout(QLayout * layout) {
	layout->setContentsMargins(0, 0, 0, 0);
	scrollFrame->setLayout(layout);
}

void BaseFrontend::onConnected(bool readOnly, QIODevice * dev) {
    (void)readOnly;
    (void)dev;
}

void BaseFrontend::onDisconnected(bool reconnecting) {
    // It is ok that there is nothing here. It is supposed to be empty.
    (void) reconnecting;
}


