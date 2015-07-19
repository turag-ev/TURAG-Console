#ifndef BASEFRONTEND_H
#define BASEFRONTEND_H

#include <QObject>
#include <QString>
#include <QWidget>
#include <QByteArray>
#include "controller.h"
#include <QList>
#include <QIcon>

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
	explicit BaseFrontend(QString name, const QIcon& icon = QIcon(), QWidget *parent = 0);
	virtual ~BaseFrontend() = default;

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
	virtual void onConnected(bool readOnly, QIODevice*);

    // frontend was disconnected from stream, does nothing if not overloaded
    virtual void onDisconnected(bool reconnecting);

public:
	void setLayout(QLayout *);

	const QIcon& getIcon() { return icon; }

protected slots:


protected:
	QFrame* scrollFrame;
	QIcon icon;
};

#endif // BASEFRONTEND_H
