#ifndef EXPANDERWIDGET_H
#define EXPANDERWIDGET_H

#include <QWidget>

class QPushButton;
class QStackedWidget;
class QVBoxLayout;

class ExpanderWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString expanderTitle READ expanderTitle WRITE setExpanderTitle STORED true)
    Q_PROPERTY(bool isExpanded READ isExpanded WRITE setExpanded STORED true)

public:
    ExpanderWidget(QWidget *parent = 0);

    QSize sizeHint() const;

	QWidget *widget(void) const;

    bool expanded = true;
    QString expanderTitle() const;
    bool isExpanded() const;

public slots:

    void buttonPressed();
    void setExpanderTitle(const QString &title);
    void setExpanded(bool flag);

	void setWidget(QWidget* widget);

signals:
    void expanderChanged(bool flag);

private:
    QPushButton *button;
    QStackedWidget *stackWidget;
    QVBoxLayout *layout;
};

#endif
