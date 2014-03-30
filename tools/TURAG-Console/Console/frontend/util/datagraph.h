#ifndef DATAGRAPH_H
#define DATAGRAPH_H

#include <qwt_plot.h>
#include <QWidget>
#include <QList>
#include <qwt_series_data.h>
#include <qwt_legend_label.h>
#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <QPalette>

class QString;
class QPointF;
class QwtPlotCurve;
class QwtPlotZoomer;
class QwtPlotItem;
class CurveDataBase;
class QwtPlotPanner;
class QSignalMapper;
class QAction;


class HoverableQwtLegendLabel : public QwtLegendLabel {
    Q_OBJECT

public:
    explicit HoverableQwtLegendLabel(QWidget *parent = 0) : QwtLegendLabel(parent) {
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
    }

    void unhighlight(void) {
        setPalette(paletteBuffer);
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

signals:
    void enter(void);
    void leave(void);

private:
    QPalette paletteBuffer;

};


class HoverableQwtLegend : public QwtLegend {
    Q_OBJECT

public:
    explicit HoverableQwtLegend(QWidget *parent=0) : QwtLegend(parent) {}

protected:
    virtual QWidget* createWidget(const QwtLegendData& data) const {
        Q_UNUSED( data );
        HoverableQwtLegendLabel* label = new HoverableQwtLegendLabel;
        label->setItemMode( defaultItemMode() );

        connect(label, SIGNAL(enter()), this, SLOT(onLabelEnter()));
        connect(label, SIGNAL(leave()), this, SLOT(onLabelLeave()));
        connect( label, SIGNAL( clicked() ), SLOT( itemClicked() ) );
        connect( label, SIGNAL( checked( bool ) ), SLOT( itemChecked( bool ) ) );

        return label;
    }

signals:
    void enter(const QVariant &itemInfo);
    void leave(const QVariant &itemInfo);

private slots:
    void onLabelEnter(void) {
        HoverableQwtLegendLabel* label = static_cast<HoverableQwtLegendLabel*>(sender());
        emit enter(itemInfo(label));
    }

    void onLabelLeave(void) {
        HoverableQwtLegendLabel* label = static_cast<HoverableQwtLegendLabel*>(sender());
        emit leave(itemInfo(label));
    }
};



class DataGraph : public QwtPlot
{
    Q_OBJECT

public:
    explicit DataGraph(QWidget *parent = 0) : DataGraph(QString(""), parent) { }
    explicit DataGraph(QString title, QWidget* parent = 0);
    ~DataGraph(void);

    int getNumberOfChannels() const;
    QString getChannelTitle(int index) const;
    QList<QAction *> getActions(void) { return actions(); }

public slots:
    virtual void clear();
    virtual void addChannel(QString title);
    virtual void addChannel(QString title, qreal timespan, bool keepHiddenPoints = true);
    virtual void addChannel(QString title,bool xAxisFixed,qreal start, qreal length, bool keepHiddenPoints = true);
    virtual void addChannel(QString title, qreal timespan, qreal y, qreal height, bool keepHiddenPoints = true);
    virtual void addChannel(QString title, qreal x, qreal width, qreal y, qreal height, bool keepHiddenPoints = true);
    virtual void removeChannel(int index);
    virtual void addData(int channel, QPointF data);
    virtual bool saveOutput(QString file);
    virtual bool saveOutput(void);
    virtual void doAutoZoom(void);
    void setZoomer(void);
    void setPanner(void);

protected:
    QList<QwtPlotCurve*> channels;
    QwtPlotZoomer* zoomer;
    QwtPlotPanner* panner;

    QAction* zoom_box_zoom_action;
    QAction* zoom_drag_action;

    virtual void updateCurveColors();
    virtual void addChannelGeneric(QString title, CurveDataBase* curveData);


protected slots:
    void showCurve(QwtPlotItem *item, bool on);
    void legendChecked(const QVariant &itemInfo, bool on);
    void onHighlightCurve(const QVariant &itemInfo);
    void onUnhighlightCurve(const QVariant &itemInfo);
    void showAllCurves(void);
    void hideAllCurves(void);


};


class CurveDataBase: public QwtArraySeriesData<QPointF>
{
protected:
    bool keepHiddenPoints_;

public:
    CurveDataBase(bool keepHiddenPoints = true) : keepHiddenPoints_(keepHiddenPoints) { }

    virtual QRectF boundingRect() const;
    virtual void append( const QPointF &point ) = 0;
    void resetBoundingRect(void) {
        d_boundingRect = CurveDataBase::boundingRect();
    }
};


class CurveData : public CurveDataBase {
public:
    CurveData() : CurveDataBase() {}
    virtual QRectF boundingRect() const;
    virtual void append( const QPointF &point );

};


class CurveDataFixedX : public CurveDataBase {
protected:
    qreal left;
    qreal right;
public:
    CurveDataFixedX(qreal x, qreal width, bool keepHiddenPoints = true) :
        CurveDataBase(keepHiddenPoints), left(x), right(x + width) { }
    virtual QRectF boundingRect() const;
    virtual void append( const QPointF &point );

};


class CurveDataFixedY : public CurveDataBase {
protected:
    qreal bottom;
    qreal top;
public:
    CurveDataFixedY(qreal y, qreal height, bool keepHiddenPoints = true) :
        CurveDataBase(keepHiddenPoints), bottom(y), top(y + height) { }
    virtual QRectF boundingRect() const;
    virtual void append( const QPointF &point );

};


class CurveDataTime : public CurveDataBase {
protected:
    qreal timespan_;
public:
    CurveDataTime(qreal timespan, bool keepHiddenPoints = true) :
        CurveDataBase(keepHiddenPoints), timespan_(timespan) { }
    virtual QRectF boundingRect() const;
    virtual void append( const QPointF &point );

};


class CurveDataTimeFixedY : public CurveDataBase {
protected:
    qreal timespan_;
    qreal bottom;
    qreal top;
public:
    CurveDataTimeFixedY(qreal timespan, qreal y, qreal height, bool keepHiddenPoints = true) :
        CurveDataBase(keepHiddenPoints), timespan_(timespan), bottom(y), top(y + height) { }
    virtual QRectF boundingRect() const;
    virtual void append( const QPointF &point );

};


class CurveDataFixedXFixedY : public CurveDataBase {
public:
    CurveDataFixedXFixedY(qreal x, qreal width, qreal y, qreal height, bool keepHiddenPoints = true);
    virtual QRectF boundingRect() const;
    virtual void append( const QPointF &point );

};




#endif // DATAGRAPH_H
