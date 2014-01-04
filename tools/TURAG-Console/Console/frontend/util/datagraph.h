#ifndef DATAGRAPH_H
#define DATAGRAPH_H

#include <qwt_plot.h>
#include <QWidget>
#include <QList>
#include <qwt_series_data.h>

class QString;
class QPointF;
class QwtPlotCurve;
class QwtPlotZoomer;
class QwtPlotItem;
class CurveDataBase;


class DataGraph : public QwtPlot
{
    Q_OBJECT

protected:
    QList<QwtPlotCurve*> channels;
    QwtPlotZoomer* zoomer;

    virtual void updateCurveColors();
    virtual void addChannelGeneric(QString title, CurveDataBase* curveData);

public:
    explicit DataGraph(QWidget *parent = 0);

    int getNumberOfChannels() const;
    QString getChannelTitle(int index) const;

protected slots:
    void showCurve(QwtPlotItem *item, bool on);
#if QWT_VERSION >= 0x060100
    void legendChecked(const QVariant &itemInfo, bool on);
#endif

public slots:
    virtual void clear();
    virtual void addChannel(QString title);
    virtual void addChannel(QString title, qreal timespan, bool keepHiddenPoints = true);
    virtual void addChannel(QString title,bool xAxisFixed,qreal start, qreal length, bool keepHiddenPoints = true);
    virtual void addChannel(QString title, qreal timespan, qreal y, qreal height, bool keepHiddenPoints = true);
    virtual void addChannel(QString title, qreal x, qreal width, qreal y, qreal height, bool keepHiddenPoints = true);
    virtual void removeChannel(int index);
    virtual void addData(int channel, QPointF data);
    virtual bool saveOutput();
};


class CurveDataBase: public QwtArraySeriesData<QPointF>
{
protected:
    bool keepHiddenPoints_;

public:
    CurveDataBase(bool keepHiddenPoints = true) : keepHiddenPoints_(keepHiddenPoints) { }

    virtual QRectF boundingRect() const;
    virtual void append( const QPointF &point ) = 0;
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
