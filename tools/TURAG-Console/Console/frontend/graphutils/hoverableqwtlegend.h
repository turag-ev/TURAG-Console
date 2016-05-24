#ifndef HOVERABLEQWTLEGEND_H
#define HOVERABLEQWTLEGEND_H

#include <qwt_legend.h>
#include "hoverableqwtlegendlabel.h"


class HoverableQwtLegend : public QwtLegend {
    Q_OBJECT

public:
    explicit HoverableQwtLegend(QWidget *parent=0) : QwtLegend(parent) {}

protected:
	virtual QWidget* createWidget(const QwtLegendData& ) const {
        HoverableQwtLegendLabel* label = new HoverableQwtLegendLabel;
        label->setItemMode( defaultItemMode() );

        connect(label, SIGNAL(enter()), this, SLOT(onLabelEnter()));
        connect(label, SIGNAL(leave()), this, SLOT(onLabelLeave()));
        connect(label, SIGNAL(mouseMiddleClicked(void)), this, SLOT(onMouseMiddleClicked(void)));
        connect( label, SIGNAL( clicked() ), SLOT( itemClicked() ) );
        connect( label, SIGNAL( checked( bool ) ), SLOT( itemChecked( bool ) ) );

        return label;
    }

signals:
    void enter(const QVariant &itemInfo);
    void leave(const QVariant &itemInfo);
    void mouseMiddleClicked(const QVariant &itemInfo);

private slots:
    void onLabelEnter(void) {
        HoverableQwtLegendLabel* label = static_cast<HoverableQwtLegendLabel*>(sender());
        emit enter(itemInfo(label));
    }

    void onLabelLeave(void) {
        HoverableQwtLegendLabel* label = static_cast<HoverableQwtLegendLabel*>(sender());
        emit leave(itemInfo(label));
    }

    void onMouseMiddleClicked(void) {
        HoverableQwtLegendLabel* label = static_cast<HoverableQwtLegendLabel*>(sender());
        emit mouseMiddleClicked(itemInfo(label));
    }
};



#endif // HOVERABLEQWTLEGEND_H
