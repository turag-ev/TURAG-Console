#ifndef TURAG_CONSOLE_CONSOLE_CONNECTIONWIDGETS_CONNECTIONWIDGET_H
#define TURAG_CONSOLE_CONSOLE_CONNECTIONWIDGETS_CONNECTIONWIDGET_H

#include <QWidget>
#include <QString>
#include <QBoxLayout>

#include <libsimeurobot/ui/common/recent.h>

class QVBoxLayout;
class QSignalMapper;
class QMenu;
class BaseBackend;
class QGroupBox;

namespace TURAG { namespace SimEurobot {
class RecentFiles;
}}

class ConnectionWidget : public QWidget {
    Q_OBJECT

public:
    static const int MAX_RECENT_CONNECTIONS = 10;

    explicit ConnectionWidget(QString recentConnectionSpecifier, QWidget *parent = 0);
    
signals:
    // the details of the connection were changed
	void connectionChanged(const QUrl& url, bool* saveConnectionString);

protected:
    TURAG::SimEurobot::RecentFiles recent_connections;
	QVBoxLayout* recentConnectionsLayout;
	QGroupBox* recentConnectionsContainer;
    QVBoxLayout* layout;

    QString recentConnectionSpecifier_;

    void addRecentConnections();
    void saveConnection(QString connectionString);

protected slots:
   virtual void onOpenRecentConnection(int index);

private:
    QSignalMapper* recent_files_map_;
	bool spacingAdded;
};


#endif // TURAG_CONSOLE_CONSOLE_CONNECTIONWIDGETS_CONNECTIONWIDGET_H
