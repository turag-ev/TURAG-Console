#ifndef TURAG_CONSOLE_CONSOLE_FRONTEND_TINAGRAPHFRONTEND_H
#define TURAG_CONSOLE_CONSOLE_FRONTEND_TINAGRAPHFRONTEND_H

#include <QStackedWidget>
#include <QList>
#include "basefrontend.h"

class QAction;
class QTreeWidget;
class QTreeWidgetItem;
class QHBoxLayout;

class TinaGraphFrontend : public BaseFrontend
{
    Q_OBJECT
public:
    explicit TinaGraphFrontend(QWidget *parent = 0);

signals:
    void newGraph(int index);

public slots:
    void writeLine(QByteArray line);
    virtual void writeData(QByteArray data);
    void activateGraph(int index);
    // frontend was connected to stream
	void onConnected(bool readOnly, QIODevice*);

    // frontend was disconnected from stream
    void onDisconnected(bool reconnecting);
    void clear(void);

private slots:
    void activateItem(QTreeWidgetItem*);

private:
   QTreeWidgetItem* createGraphEntry(int index, const QString& title);


    QList<int> graphIndices;
    QStackedWidget* stack;
    QTreeWidget* graphList;
	QHBoxLayout* layout;
};

#endif // TURAG_CONSOLE_CONSOLE_FRONTEND_TINAGRAPHFRONTEND_H
