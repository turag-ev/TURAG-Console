#ifndef TURAG_CONSOLE_CONSOLE_FRONTEND_GRAPHUTILS_DATAGRAPH_H
#define TURAG_CONSOLE_CONSOLE_FRONTEND_GRAPHUTILS_DATAGRAPH_H

#include <QWidget>
#include <QList>
#include <qwt_series_data.h>
#include <qwt_plot_curve.h>
#include <QPalette>
#include <QTimer>
#include <QSignalMapper>
#include <libs/splitterext.h>
#include <QSplitter>
#include <qwt_plot.h>
#include <qwt_text.h>

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
class QwtPlotMarker;




class DataGraph : public QSplitter
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
    void setTitle (const QString & title) {
        plot->setTitle(title);
    }
	QString getTitle(void) {
		return plot->title().text();
	}

    void addChannelGroup(const QList<int>& channelGroup);
	void applyChannelGrouping(int index, bool showCurves = true);
    void resetChannelGrouping(void);

	/**
	 * @brief Übernimmt Metadaten von einer anderen Instanz.
	 * @param source Referenz auf die Quelle der Metadaten.
	 *
	 * Die übernommenen Metadaten umfassen Titel, Channel und
	 * Channelgruppen.
	 */
	void copyMetadata(DataGraph& source);

public slots:
	/**
	 * @brief Löscht sämtliche Daten und Metadaten des Diagramms.
	 *
	 * Neben den Diagrammdaten werden Channels und Channelgruppen
	 * gelöscht.
	 */
    virtual void clear();

	/**
	 * @brief Löscht alle Daten des Diagramms.
	 *
	 * Metadaten wie Channel und Channelgruppen bleiben erahlten.
	 */
	virtual void clearData(void);

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
    virtual void addVerticalMarker(float time);
    virtual void focusVerticalMarker(unsigned index);
    void setUserInputMode(int index);
    void requestReplot(void);
    void execReplot(void);

protected:
    QList<QwtPlotCurve*> channels;
    QList<QwtPlotMarker*> vMarkers;
    QwtPlotZoomer* zoomer;
    QwtPlotPanner* panner;
    CanvasPicker* picker;
    QList< QList<int> > channelGroups;

    virtual void updateCurveColors();
    virtual void addChannelGeneric(QString title, CurveDataBase* curveData);
    void updateRightAxis(void);
    void applyCurveStyleToCurve(QwtPlotCurve* curve);


protected slots:
    void showCurve(QwtPlotItem *item, bool on);
    void showCurve(QwtPlotItem *item, bool on, bool visible);
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
	void createDataTable(void);

    QAction* show_datatable_action;
    int curvesWithRightYAxis;

    QSignalMapper styleMapper;
    QSignalMapper userInputModeMapper;
    int selectedStyle;

    QTableWidget* dataTable;
    QwtPlot* plot;
    QComboBox* dataTableChannelList;

	QTimer refreshTimer;
	bool updateCurveColorsRequested;

	int appliedChannelGrouping;
};


class CurveDataBase: public QwtArraySeriesData<QPointF>
{
protected:
	bool keepHiddenPoints_;

public:
    CurveDataBase(bool keepHiddenPoints = true) : keepHiddenPoints_(keepHiddenPoints) { }

    virtual QRectF boundingRect() const;
    virtual void append( const QPointF &point ) = 0;
	virtual void resetBoundingRect(void) {
		// invalidate bounding rect
		cachedBoundingRect = QRectF();
		// retrieve new bounding rect
		cachedBoundingRect = boundingRect();
    }
	void clear(void) {
		m_samples.clear();
	}
	virtual CurveDataBase* createInstance(void) = 0;
};


class CurveData : public CurveDataBase {
public:
	CurveData(bool keepHiddenPoints = true) : CurveDataBase(keepHiddenPoints) {}
    virtual QRectF boundingRect() const;
    virtual void append( const QPointF &point );
	virtual CurveDataBase* createInstance(void) {
		return new CurveData(keepHiddenPoints_);
	}
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
	virtual CurveDataBase* createInstance(void) {
		return new CurveDataFixedX(left, right - left, keepHiddenPoints_);
	}
	virtual void resetBoundingRect(void) {
		cachedBoundingRect = CurveDataBase::boundingRect();
	}
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
	virtual CurveDataBase* createInstance(void) {
		return new CurveDataFixedY(bottom, top - bottom, keepHiddenPoints_);
	}
};


class CurveDataTime : public CurveDataBase {
protected:
    qreal timespan_;
public:
	CurveDataTime(qreal timespan, bool keepHiddenPoints = true) :
		CurveDataBase(keepHiddenPoints), timespan_(timespan) { }
	virtual QRectF boundingRect() const;
    virtual void append( const QPointF &point );
	virtual CurveDataBase* createInstance(void) {
		return new CurveDataTime(timespan_, keepHiddenPoints_);
	}
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
	virtual CurveDataBase* createInstance(void) {
		return new CurveDataTimeFixedY(timespan_, bottom, top - bottom, keepHiddenPoints_);
	}
};


class CurveDataFixedXFixedY : public CurveDataBase {
public:
	CurveDataFixedXFixedY(qreal x, qreal width, qreal y, qreal height, bool keepHiddenPoints = true) :
		CurveDataBase(keepHiddenPoints), bottom(y), top(y + height), left(x), right(x + width) { }
	virtual QRectF boundingRect() const;
    virtual void append( const QPointF &point );
	virtual CurveDataBase* createInstance(void) {
		return new CurveDataFixedXFixedY(left, right - left, bottom, top - bottom, keepHiddenPoints_);
	}

protected:
	qreal bottom;
	qreal top;
	qreal left;
	qreal right;
};




#endif // TURAG_CONSOLE_CONSOLE_FRONTEND_GRAPHUTILS_DATAGRAPH_H
