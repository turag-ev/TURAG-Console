#ifndef FASTLISTWIDGET_H
#define FASTLISTWIDGET_H

#include <QListWidget>
#include <QTimer>
#include <QListWidgetItem>
#include <QList>
#include <QScrollBar>

class FastListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit FastListWidget(QWidget *parent = 0) :
        QListWidget(parent)
    {
        updateTimer.setInterval(100);
        connect(&updateTimer, SIGNAL(timeout()), this, SLOT(insertBufferedItems()));
    }

    inline void addItem(QListWidgetItem *item) {
        itemBuffer.append(item);

        if (!updateTimer.isActive()) {
            updateTimer.start();
        }
    }

private slots:
    void insertBufferedItems(void) {
        updateTimer.stop();

        for (QListWidgetItem* item : itemBuffer) {
            QListWidget::addItem(item);
        }
        itemBuffer.clear();

        QScrollBar* scrollbar = verticalScrollBar();
        if (scrollbar->value() == scrollbar->maximum()) {
            scrollToBottom();
        }
    }

private:
    QTimer updateTimer;
    QList<QListWidgetItem*> itemBuffer;

};

#endif // FASTLISTWIDGET_H
