#include "basefrontend.h"
#include <QList>
#include <QAction>
#include <QMenu>
#include <QPoint>
#include <QScrollArea>
#include <QFrame>
#include <QHBoxLayout>

BaseFrontend::BaseFrontend(QString name, QWidget *parent) :
	QWidget(parent)
{
    setObjectName(name);

    contextMenu = new QMenu(this);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showContextMenu(QPoint)));


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

void BaseFrontend::onConnected(bool readOnly, bool isBuffered, QIODevice * dev) {
    (void)readOnly;
    (void)isBuffered;
    (void)dev;
}

void BaseFrontend::onDisconnected(bool reconnecting) {
    // It is ok that there is nothing here. It is supposed to be empty.
    (void) reconnecting;
}



void BaseFrontend::setExternalContextActions(QList<QAction*> actions) {
    externalContextActions = actions;
    rebuildContextMenu();
}


void BaseFrontend::addAction(QAction * action) {
    QWidget::addAction(action);
    rebuildContextMenu();
}

void BaseFrontend::addActions(QList<QAction *> actions) {
    QWidget::addActions(actions);
    rebuildContextMenu();
}

void BaseFrontend::insertAction(QAction * before, QAction * action) {
    QWidget::insertAction(before, action);
    rebuildContextMenu();
}

void BaseFrontend::insertActions(QAction * before, QList<QAction *> actions) {
    QWidget::insertActions(before, actions);
    rebuildContextMenu();
}

void BaseFrontend::removeAction(QAction * action) {
    QWidget::removeAction(action);
    rebuildContextMenu();
}

void BaseFrontend::clearActions(void) {
    while(QWidget::actions().count()) {
        QWidget::removeAction(QWidget::actions().at(0));
    }
}

void BaseFrontend::rebuildContextMenu() {
    contextMenu->clear();
    contextMenu->addActions(externalContextActions);
    contextMenu->addActions(actions());
}

void BaseFrontend::showContextMenu(const QPoint &pos) {
    QPoint globalPos = mapToGlobal(pos);
    contextMenu->popup(globalPos);
}
