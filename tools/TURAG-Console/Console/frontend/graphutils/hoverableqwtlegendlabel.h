#ifndef HOVERABLEQWTLEGENDLABEL_H
#define HOVERABLEQWTLEGENDLABEL_H

#include <qwt_legend_label.h>
#include <QMouseEvent>


class HoverableQwtLegendLabel : public QwtLegendLabel {
    Q_OBJECT

public:
    explicit HoverableQwtLegendLabel(QWidget *parent = 0) :
        QwtLegendLabel(parent), highlighted(false) {
        setAutoFillBackground(true);
        setBackgroundRole(QPalette::Base);
    }

public slots:
    void highlight(void) {
        paletteBuffer = palette();
        QPalette p = palette();
        p.setColor(QPalette::Button, Qt::white);
        p.setColor(QPalette::Base, Qt::black);
        p.setColor(QPalette::Text, Qt::white);
        setPalette(p);
        highlighted = true;
    }

    void unhighlight(void) {
        setPalette(paletteBuffer);
        highlighted = false;
    }

    void setRightSide(void) {
        QPalette p = palette();
        p.setColor(QPalette::Text, Qt::blue);
        setPalette(p);
        paletteBuffer.setColor(QPalette::Text, Qt::blue);
    }

    void setLeftSide(void) {
        QPalette p = palette();
        if (highlighted) {
            p.setColor(QPalette::Text, Qt::white);
        } else {
            p.setColor(QPalette::Text, Qt::black);
        }
        setPalette(p);
        paletteBuffer.setColor(QPalette::Text, Qt::black);
    }

protected:
    virtual void enterEvent ( QEvent * event ) {
        QWidget::enterEvent(event);
        emit enter();
    }

    virtual void leaveEvent ( QEvent * event ) {
        QWidget::leaveEvent(event);
        emit leave();
    }

    virtual void mousePressEvent(QMouseEvent * event) {
        if (event->button() == Qt::MiddleButton || (event->button() == Qt::LeftButton && (event->modifiers() & Qt::ControlModifier) )) {
            emit mouseMiddleClicked();
        } else {
            QwtLegendLabel::mousePressEvent(event);
        }
    }

signals:
    void enter(void);
    void leave(void);
    void mouseMiddleClicked(void);

private:
    QPalette paletteBuffer;
    bool highlighted;

};



#endif // HOVERABLEQWTLEGENDLABEL_H
