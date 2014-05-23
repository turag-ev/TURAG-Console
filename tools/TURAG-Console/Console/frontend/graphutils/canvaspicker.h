#include <QObject>
#include <qwt_widget_overlay.h>
#include <QRegion>
#include <qwt_symbol.h>

class QPoint;
class QCustomEvent;
class QwtPlot;
class QwtPlotCurve;


class SymbolOverlay : public QwtWidgetOverlay {
    Q_OBJECT

public:
    explicit SymbolOverlay(int size, QWidget * widget);

    void showSymbol(const QPointF & pos_);
    void hideSymbol(void);

protected:
    virtual QRegion maskHint() const;
    virtual void drawOverlay(QPainter *painter) const;

private:
    bool show;
    QPointF pos;
    QwtSymbol* symbol;
    QwtSymbol* shadow;
};


class CanvasPicker: public QObject
{
    Q_OBJECT
public:
    CanvasPicker( QwtPlot *plot );
    virtual bool eventFilter( QObject *, QEvent * );

    void setEnabled(bool enabled);

public slots:
    void selectPlotCurve(QwtPlotCurve* curve);
    void selectPlotPoint(int index);

signals:
    void dataPointSuggested(int index);
    void nextPlotCurveSuggested(void);
    void previuosPlotCurveSuggested(void);

private:
    void selectManually( const QPoint & );

    void showCursor(bool show );
    void shiftPointCursor(int amount );

    QwtPlot *plot();
    const QwtPlot *plot() const;

    QwtPlotCurve *d_selectedCurve;
    int d_selectedPoint;

    SymbolOverlay* overlay;
};


