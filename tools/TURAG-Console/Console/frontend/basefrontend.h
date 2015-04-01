#ifndef BASEFRONTEND_H
#define BASEFRONTEND_H

#include <QObject>
#include <QString>
#include <QWidget>
#include <QByteArray>
#include "controller.h"
#include <QList>

class QAction;
class QMenu;
class QPoint;
class QLayout;
class QFrame;
class QScrollArea;


class BaseFrontend : public QWidget
{
    Q_OBJECT

public:
    explicit BaseFrontend(QString name, QWidget *parent = 0);

    // This function is called from the parent widget. Its purpose
    // to provide the context entries from parent widgets in child
    // widgets as well. If you override this fucntion you should always
    // call the base implementation.
    virtual void setExternalContextActions(QList<QAction*> actions);

signals:
    // data was received from the frontend (usually user input)
    void dataReady(QByteArray data);

    // request new data from backend (if connected)
    void requestData();


public slots:
    // this slot is called by the controller when new data is available
    virtual void writeData(QByteArray data) = 0;
    virtual void clear(void) = 0;

    // frontend was connected to stream, does nothing if not overloaded
    virtual void onConnected(bool readOnly, bool isBuffered, QIODevice*);

    // frontend was disconnected from stream, does nothing if not overloaded
    virtual void onDisconnected(bool reconnecting);

public:
	void setLayout(QLayout *);

protected slots:
    void showContextMenu(const QPoint & pos);


protected:
    void rebuildContextMenu();

    // Use these functions to add menu actions. They will be automatically
    // shown in the right places.
    void addAction(QAction * action);
    void addActions(QList<QAction *> actions);
    void insertAction(QAction * before, QAction * action);
    void insertActions(QAction * before, QList<QAction *> actions);
    void removeAction(QAction * action);
    void clearActions(void);


    QList<QAction*> externalContextActions;
	QMenu* contextMenu;
	QFrame* scrollFrame;
};

#endif // BASEFRONTEND_H
