#ifndef DATAGRAPH_H
#define DATAGRAPH_H

#include <QWidget>
#include <QList>
#include <qwt_series_data.h>
#include <qwt_plot_curve.h>
#include <QPalette>
#include <QTimer>
#include <QSignalMapper>
#include <libs/splitterext.h>
#include <qwt_plot.h>

class QString;
class QPointF;
class QwtPlotCurve;
class QwtPlotZoomer;
class QwtPlotItem;
class CurveDataBase;
class QwtPlotPanner;
class QSignalMapper;
class CheckActionExt;
class QTableWidget;
class CanvasPicker;
class QComboBox;




class DataGraph : public SplitterExt
{
    Q_OBJECT

public:
    enum class Style {
        symbols,
        interpolated_line,
        stepped_line,
        interpolated_line_and_symbols,
        stepped_line_and_symbols,
    };

    explicit DataGraph(QWidget *parent = 0) : DataGraph(QString(""), parent) { }
    explicit DataGraph(QString title, QWidget* parent = 0);
    ~DataGraph(void);

    int getNumberOfChannels() const;
    QString getChannelTitle(int index) const;
    QList<QAction *> getActions(void) { return actions(); }
    void setTitle (const QString & title) {
        plot->setTitle(title);
    }

public slots:
    virtual void clear();
    virtual void addChannel(QString title);
    virtual void addChannel(QString title, qreal timespan, bool keepHiddenPoints = true);
    virtual void addChannel(QString title,bool xAxisFixed,qreal start, qreal length, bool keepHiddenPoints = true);
    virtual void addChannel(QString title, qreal timespan, qreal y, qreal height, bool keepHiddenPoints = true);
    virtual void addChannel(QString title, qreal x, qreal width, qreal y, qreal height, bool keepHiddenPoints = true);
    virtual void removeChannel(int index);
    virtual void addData(int channel, QPointF data);
    virtual bool exportOutput(QString file);
    virtual bool exportOutput(void);
    virtual void doAutoZoom(void);
    void setUserInputMode(int index);

protected:
    QList<QwtPlotCurve*> channels;
    QwtPlotZoomer* zoomer;
    QwtPlotPanner* panner;
    CanvasPicker* picker;

    virtual void updateCurveColors();
    virtual void addChannelGeneric(QString title, CurveDataBase* curveData);
    void updateRightAxis(void);
    void applyCurveStyleToCurve(QwtPlotCurve* curve);


protected slots:
    void showCurve(QwtPlotItem *item, bool on);
    void legendChecked(const QVariant &itemInfo, bool on);
    void legendMouseMiddleClicked(const QVariant &itemInfo);
    void onHighlightCurve(const QVariant &itemInfo);
    void onUnhighlightCurve(const QVariant &itemInfo);
    void showAllCurves(void);
    void hideAllCurves(void);
    void setCurveStyle(int styleIndex);
    void showDataTable(bool show);
    void generateDataTableForChannel(int index);
    void showEntryInDatatable(int index);
    void selectNextPlotCurve(void);
    void selectPreviousPlotCurve(void);

private:
    void addEntryToDataTable(QPointF data, int row = -1);

    QAction* show_datatable_action;
    QTimer refreshTimer;
    int curvesWithRightYAxis;

    QSignalMapper styleMapper;
    QSignalMapper userInputModeMapper;
    int selectedStyle;

    QTableWidget* dataTable;
    QwtPlot* plot;
    QComboBox* dataTableChannelList;
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
