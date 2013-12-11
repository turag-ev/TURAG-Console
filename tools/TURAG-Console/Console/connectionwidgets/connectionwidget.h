#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <QWidget>
#include <QString>
#include <QBoxLayout>

class QGroupBox;
class QVBoxLayout;
class QSignalMapper;
class QMenu;

class ConnectionWidget : public QWidget {
    Q_OBJECT

public:
    static const int MAX_RECENT_CONNECTIONS = 5;

    explicit ConnectionWidget(QString recentConnectionSpecifier, QWidget *parent = 0);
    virtual QMenu* getMenu();
    
signals:
    // the details of the connection were changed
    void connectionChanged(QString connectionString, bool* saveConnectionString);

protected:
    QStringList recent_connections;
    QGroupBox* recentConnectionsContainer;
    QVBoxLayout* layout;

    QString recentConnectionSpecifier_;

    void addRecentConnections();
    void saveConnection(QString connectionString);

protected slots:
    void onOpenRecentConnection(int index);

private:
    QSignalMapper* recent_files_map_;
};


#endif // CONNECTIONWIDGET_H
