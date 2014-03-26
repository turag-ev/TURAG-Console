#include "basefrontend.h"
#include <QList>
#include <QAction>
#include <QMenu>
#include <QPoint>

BaseFrontend::BaseFrontend(QString name, QWidget *parent) :
    QWidget(parent)
{
    setObjectName(name);

    contextMenu = new QMenu(this);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showContextMenu(QPoint)));
}


void BaseFrontend::onConnected(bool readOnly, bool isSequential, QIODevice * dev) {
    (void)readOnly;
    (void)isSequential;
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
