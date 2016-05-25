#include "connectionwidget.h"

#include <QSettings>
#include <QSignalMapper>
#include <QStringList>
#include <QLabel>
#include <QGroupBox>
#include <QMenu>
#include <QGroupBox>
#include <QUrl>

#include "libs/elidedbutton.h"

ConnectionWidget::ConnectionWidget(QString recentConnectionSpecifier, QWidget *parent) :
	QWidget(parent), recentConnectionsLayout(nullptr), recentConnectionsContainer(nullptr),
    recentConnectionSpecifier_(recentConnectionSpecifier), spacingAdded(false)
{
    recent_connections.setMaxNumber(MAX_RECENT_CONNECTIONS);

    recent_files_map_ = new QSignalMapper(this);
    connect(recent_files_map_,SIGNAL(mapped(int)),this,SLOT(onOpenRecentConnection(int)));

    layout = new QVBoxLayout(this);
}

void ConnectionWidget::addRecentConnections() {
	// remove recentConnections in case this is not the first call of this function
	if (recentConnectionsLayout != nullptr) {
		layout->removeItem(recentConnectionsLayout);
		if (recentConnectionsContainer != nullptr) {
			recentConnectionsLayout->removeWidget(recentConnectionsContainer);
			recentConnectionsContainer->deleteLater();
		}
		recentConnectionsLayout->deleteLater();
	}

	recentConnectionsLayout = new QVBoxLayout;

    QSettings settings;
    settings.beginGroup("RecentConnections");
    recent_connections.loadFromSettings(objectName(), settings);

    if (!recent_connections.isEmpty()) {
		recentConnectionsContainer = new QGroupBox(recentConnectionSpecifier_ + ":");
		recentConnectionsContainer->setFlat(true);
		QVBoxLayout* innerLayout = new QVBoxLayout;

        for (int i = 0; i < recent_connections.getSize(); i++) {
            ElidedButton* link = new ElidedButton(" " + recent_connections[i]);
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
		recentConnectionsLayout->addWidget(recentConnectionsContainer);
	}
	// Two variants here: place it directly beneath the other widgets with a fixed
	// spacing:
	if (!spacingAdded) {
		layout->addSpacing(25);
		spacingAdded = true;
	}
	recentConnectionsLayout->addStretch();

	// or place it at the bottom:
//	layout->addStretch();

	// I think the first variant is preferrable because
	// it keeps the layout more compact.


	layout->addLayout(recentConnectionsLayout);

}

void ConnectionWidget::onOpenRecentConnection(int index)
{
    bool save;
    emit connectionChanged(QUrl(recent_connections[index]), &save);
    if (save) {
        saveConnection(recent_connections[index]);
        addRecentConnections();
    }
}


void ConnectionWidget::saveConnection(QString connectionString)
{
    recent_connections.add(connectionString);

    QSettings settings;
    settings.beginGroup("RecentConnections");
    recent_connections.saveToSettings(objectName(), settings);
}

