#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <QWidget>
#include <QString>
#include <QBoxLayout>

class QGroupBox;
class QVBoxLayout;


class ConnectionWidget : public QWidget
{
    Q_OBJECT

protected:
    QStringList recent_connections;
    QGroupBox* recentConnectionsContainer;
    QVBoxLayout* layout;

    QString recentConnectionSpecifier_;

    virtual void addRecentConnections();
    virtual void saveConnection(QString connectionString);

protected slots:
    void onOpenRecentConnection(int index);

public:
    static const int MAX_RECENT_CONNECTIONS = 5;

    explicit ConnectionWidget(QString recentConnectionSpecifier, QWidget *parent = 0);
    
signals:
    // the details of the connection were changed
    void connectionChanged(QString connectionString, bool* saveConnectionString);
    
};


#endif // CONNECTIONWIDGET_H
