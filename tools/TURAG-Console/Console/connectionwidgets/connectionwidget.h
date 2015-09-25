#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <QWidget>
#include <QString>
#include <QBoxLayout>

class QVBoxLayout;
class QSignalMapper;
class QMenu;
class BaseBackend;
class QGroupBox;

class ConnectionWidget : public QWidget {
    Q_OBJECT

public:
    static const int MAX_RECENT_CONNECTIONS = 10;

    explicit ConnectionWidget(QString recentConnectionSpecifier, QWidget *parent = 0);
    
signals:
    // the details of the connection were changed
	void connectionChanged(const QUrl& url, bool* saveConnectionString);

protected:
    QStringList recent_connections;
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


#endif // CONNECTIONWIDGET_H
