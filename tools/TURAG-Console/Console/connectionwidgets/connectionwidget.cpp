#include "connectionwidget.h"

#include <QSettings>
#include <QSignalMapper>
#include <QStringList>
#include <QLabel>
#include <QGroupBox>

#include "libs/elidedbutton.h"


ConnectionWidget::ConnectionWidget(QString recentConnectionSpecifier, QWidget *parent) :
    QWidget(parent), recentConnectionsContainer(nullptr), recentConnectionSpecifier_(recentConnectionSpecifier)
{
    recent_files_map_ = new QSignalMapper(this);
    connect(recent_files_map_,SIGNAL(mapped(int)),this,SLOT(onOpenRecentConnection(int)));
}

void ConnectionWidget::addRecentConnections() {
    // remove recentCOnnections in case this is not the first call of this function
    if (recentConnectionsContainer != nullptr) {
        layout->removeWidget(recentConnectionsContainer);
        delete recentConnectionsContainer;
    }

    QSettings settings;
    settings.beginGroup("RecentConnections");
    recent_connections = settings.value(recentConnectionSpecifier_).toStringList();

    if (!recent_connections.empty()) {
        recentConnectionsContainer = new QGroupBox(recentConnectionSpecifier_ + ":");
        QVBoxLayout* innerLayout = new QVBoxLayout;

        for (int i = 0; i < recent_connections.length(); i++) {
            ElidedButton* link = new ElidedButton(recent_connections[i]);
            link->setFlat(true);
            link->setElideMode(Qt::ElideMiddle);
            link->setMinimumWidth(350);
            link->setMaximumWidth(1024);
            link->setToolTip(recent_connections[i]);
            link->setStyleSheet(QString::fromUtf8("text-align:left;"));

            connect(link, SIGNAL(clicked()), recent_files_map_, SLOT(map()));
            recent_files_map_->setMapping(link, i);
            innerLayout->addWidget(link, 0, Qt::AlignLeft);
        }
        recentConnectionsContainer->setLayout(innerLayout);
        layout->addWidget(recentConnectionsContainer);
    }
}

void ConnectionWidget::onOpenRecentConnection(int index) {
    bool save;
    emit connectionChanged(recent_connections.at(index), &save);
    if (save) {
        saveConnection(recent_connections.at(index));
        addRecentConnections();
    }
}


void ConnectionWidget::saveConnection(QString connectionString) {
    QSettings settings;
    settings.beginGroup("RecentConnections");

    QStringList connections = settings.value(recentConnectionSpecifier_).toStringList();
    connections.removeAll(connectionString);
    connections.prepend(connectionString);
    while (connections.size() > MAX_RECENT_CONNECTIONS) {
        connections.removeLast();
    }

    settings.setValue(recentConnectionSpecifier_, connections);
}
