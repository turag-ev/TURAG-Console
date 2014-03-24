#include "datagraph.h"
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_zoomer.h>
#include <qwt_interval.h>
#include <QPen>
#include <QColor>
#include <QImageWriter>
#include <QList>
#include <QStringList>
#include <QByteArray>
#include <QFileDialog>
#include "colormap.h"
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <QDebug>
#include <QVector>

#if QWT_VERSION < 0x060100
# include <qwt_legend_item.h>
#else
# include <qwt_legend_label.h>
#endif

class CurveDataBase;
class CurveData;


// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// class DataGraph
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

DataGraph::DataGraph(QString title, QWidget *parent) :
    QwtPlot(title, parent)
{
    panner = new QwtPlotPanner( canvas() );
    zoomer = new QwtPlotZoomer(canvas());

    // zoom in/out with the wheel
    QwtPlotMagnifier* magnifier = new QwtPlotMagnifier( canvas() );
    magnifier->setWheelFactor(1.2);
    magnifier->setMouseButton(Qt::NoButton);


//    setTitle("A Simple QwtPlot Demonstration");
    QwtLegend *legend = new QwtLegend;
    insertLegend(legend, QwtPlot::BottomLegend);

#if QWT_VERSION < 0x060100
    legend->setItemMode(QwtLegend::CheckableItem);
    connect(this, SIGNAL(legendChecked(QwtPlotItem *, bool)), SLOT(showCurve(QwtPlotItem *, bool)));
#else
    legend->setDefaultItemMode( QwtLegendData::Checkable );
    connect(legend, SIGNAL(checked(const QVariant &, bool, int)), SLOT(legendChecked(const QVariant &, bool)));
#endif

    QwtPlotPicker* d_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, canvas());
//    d_picker->setStateMachine(new QwtPickerDragPointMachine());
    d_picker->setRubberBandPen(QColor(Qt::lightGray));
    d_picker->setRubberBand(QwtPicker::CrossRubberBand);
    d_picker->setTrackerPen(QColor(Qt::black));



    // axes
//    setAxisTitle(xBottom, "x -->" );
//    setAxisScale(xBottom, 0.0, 10.0);

//    setAxisTitle(yLeft, "y -->");
//    setAxisScale(yLeft, -1.0, 1.0);

    // canvas
    static_cast<QwtPlotCanvas*>(canvas())->setLineWidth(0);


    QPalette canvasPalette( Qt::white );
    canvasPalette.setColor( QPalette::Foreground, QColor( 133, 190, 232 ) );
    canvas()->setPalette( canvasPalette );
}


int DataGraph::getNumberOfChannels() const {
    return channels.size();
}

QString DataGraph::getChannelTitle(int index) const {
    if (index < channels.size()) {
        return channels.at(index)->title().text();
    } else {
        return "";
    }
}

void DataGraph::addChannelGeneric(QString title, CurveDataBase* curveData) {
    QwtPlotCurve *curve = new QwtPlotCurve(title);
    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve->setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
    curve->attach(this);
    curve->setData(curveData);

    channels.append(curve);
    updateCurveColors();
    showCurve(curve, true);

    //    curve->setPen(QPen(Qt::red));
}

void DataGraph::addChannel(QString title) {
    addChannelGeneric(title, new CurveData);
}

void DataGraph::addChannel(QString title, qreal timespan, bool keepHiddenPoints) {
    addChannelGeneric(title, new CurveDataTime(timespan, keepHiddenPoints));
}

void DataGraph::addChannel(QString title, bool xAxisFixed, qreal start, qreal length, bool keepHiddenPoints) {
    if (xAxisFixed) {
        addChannelGeneric(title, new CurveDataFixedX(start, length, keepHiddenPoints));
    } else {
        addChannelGeneric(title, new CurveDataFixedY(start, length, keepHiddenPoints));
    }
}

void DataGraph::addChannel(QString title, qreal timespan, qreal y, qreal height, bool keepHiddenPoints) {
    addChannelGeneric(title, new CurveDataTimeFixedY(timespan, y, height, keepHiddenPoints));
}

void DataGraph::addChannel(QString title, qreal x, qreal width, qreal y, qreal height, bool keepHiddenPoints) {
    addChannelGeneric(title, new CurveDataFixedXFixedY(x, width, y, height, keepHiddenPoints));
}


void DataGraph::removeChannel(int index) {
    if (index < channels.size()) {
        channels.at(index)->detach();
        delete channels.at(index);
        channels.removeAt(index);
    }
    updateCurveColors();
    replot();
}

void DataGraph::clear() {
    for (auto iter : channels) {
        iter->detach();
        delete iter;
    }
    channels.clear();
    replot();
}

void DataGraph::setZoomer(void) {
   zoomer->setEnabled(true);
   panner->setEnabled(false);
}

void DataGraph::setPanner(void) {
    zoomer->setEnabled(false);
    panner->setEnabled(true);
}



void DataGraph::addData(int channel, QPointF data) {
    if (channel < channels.size()) {
        CurveDataBase *curvedata = static_cast<CurveData *>( channels.at(channel)->data() );
        curvedata->append(data);
    } else {
        (void) data;
    }

    setAxisAutoScale(xBottom, true);
    setAxisAutoScale(yLeft, true);
    replot();
}

void DataGraph::doAutoZoom(void) {

    for (QwtPlotCurve* channel : channels) {
        CurveDataBase *curvedata = static_cast<CurveData *>( channel->data() );
        curvedata->resetBoundingRect();
    }
    setAxisAutoScale(xBottom, true);
    setAxisAutoScale(yLeft, true);
    replot();
}

#if QWT_VERSION < 0x060100
void DataGraph::showCurve(QwtPlotItem *item, bool on) {
    item->setVisible(on);
    QWidget *w = legend()->find(item);
    if ( w && w->inherits("QwtLegendItem") )
        ((QwtLegendItem *)w)->setChecked(on);

    replot();
}
#else
void DataGraph::legendChecked(const QVariant &itemInfo, bool on) {
    QwtPlotItem *plotItem = infoToItem(itemInfo);
    if (plotItem) {
        showCurve(plotItem, on);
    }
}

void DataGraph::showCurve(QwtPlotItem *item, bool on) {
    item->setVisible(on);

    QwtLegend *lgd = qobject_cast<QwtLegend *>(legend());

    QList<QWidget *> legendWidgets = lgd->legendWidgets(itemToInfo(item));

    if (legendWidgets.size() == 1) {
        QwtLegendLabel *legendLabel = qobject_cast<QwtLegendLabel *>(legendWidgets[0]);

        if (legendLabel) {
            legendLabel->setChecked(on);
        }
    }
    replot();
}
#endif


void DataGraph::updateCurveColors() {
    ColorMapDiscrete colormap(channels.size());

    for (int i = 0; i < channels.size(); ++i) {
        channels.at(i)->setPen(QPen(colormap.getColor(i)));
    }
}


bool DataGraph::saveOutput() {
    QString fileName = "plot.png";

    QStringList filter;
    filter += "PNG-Images (*.png)";
    filter += "JPEG-Images (*.jpg)";
    filter += "PDF Documents (*.pdf)";
#ifndef QWT_NO_SVG
    filter += "SVG Documents (*.svg)";
#endif
    filter += "Postscript Documents (*.ps)";

    fileName = QFileDialog::getSaveFileName(this, "Export File Name", fileName,
        filter.join(";;"), NULL);


    if ( !fileName.isEmpty() )
    {
        QwtPlotRenderer renderer;

        // flags to make the document look like the widget
        renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, true);
        renderer.setLayoutFlag(QwtPlotRenderer::DefaultLayout, true);

        renderer.renderDocument(this, fileName, QSizeF(300, 200), 85);

        return true;
    } else {
        return false;
    }
}



// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// CurveDataBase
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
QRectF CurveDataBase::boundingRect() const {
    QRectF rect;

    if (d_samples.size()) {
        rect.setRight(d_samples[0].x());
        rect.setLeft(d_samples[0].x());
        rect.setTop(d_samples[0].y());
        rect.setBottom(d_samples[0].y());
    }

    for (int i = 1; i < d_samples.size(); ++i) {
        const QPointF& point = d_samples[i];
        if (point.x() > rect.right()) {
            rect.setRight(point.x());
        } else if (point.x() < rect.left()) {
            rect.setLeft(point.x());
        }
        if (point.y() > rect.bottom()) {
            rect.setBottom(point.y());
        } else if (point.y() < rect.top()) {
            rect.setTop(point.y());
        }
    }

    if (rect.height() == 0) {
        rect.setHeight(1);
    }
    if (rect.width() == 0) {
        rect.setWidth(1);
    }
    return rect;
}


// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// CurveData
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
QRectF CurveData::boundingRect() const {
    if (!d_boundingRect.isValid()) {
        d_boundingRect = CurveDataBase::boundingRect();
    }
    return d_boundingRect;
}


void CurveData::append( const QPointF &point ) {
    d_samples += point;

    if (d_boundingRect.isValid()) {
        if (point.x() > d_boundingRect.right()) {
            d_boundingRect.setRight(point.x());
        } else if (point.x() < d_boundingRect.left()) {
            d_boundingRect.setLeft(point.x());
        }
        if (point.y() > d_boundingRect.bottom()) {
            d_boundingRect.setBottom(point.y());
        } else if (point.y() < d_boundingRect.top()) {
            d_boundingRect.setTop(point.y());
        }
    }
}

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// CurveDataFixedX
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
QRectF CurveDataFixedX::boundingRect() const {
    if (!d_boundingRect.isValid()) {
        d_boundingRect = CurveDataBase::boundingRect();
        d_boundingRect.setLeft(left);
        d_boundingRect.setRight(right);
    }
    return d_boundingRect;
}

void CurveDataFixedX::append( const QPointF &point ) {
    if (keepHiddenPoints_|| !d_boundingRect.isValid() || (point.x() >= left && point.x() <= right)) {
        d_samples += point;

        if (d_boundingRect.isValid()) {
            if (point.y() > d_boundingRect.bottom()) {
                d_boundingRect.setBottom(point.y());
            } else if (point.y() < d_boundingRect.top()) {
                d_boundingRect.setTop(point.y());
            }
        }
    }
}


// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// CurveDataFixedY
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
QRectF CurveDataFixedY::boundingRect() const {
    if (!d_boundingRect.isValid()) {
        d_boundingRect = CurveDataBase::boundingRect();
        d_boundingRect.setBottom(top);
        d_boundingRect.setTop(bottom);
    }
    return d_boundingRect;
}

void CurveDataFixedY::append( const QPointF &point ) {
    if (keepHiddenPoints_|| !d_boundingRect.isValid() || (point.y() >= bottom && point.y() <= top)) {
        d_samples += point;

        if (d_boundingRect.isValid()) {
            if (point.x() > d_boundingRect.right()) {
                d_boundingRect.setRight(point.x());
            } else if (point.x() < d_boundingRect.left()) {
                d_boundingRect.setLeft(point.x());
            }
        }
    }
}



// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// CurveDataTime
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
QRectF CurveDataTime::boundingRect() const {
    if (!d_boundingRect.isValid()) {
        d_boundingRect = CurveDataBase::boundingRect();
        if (d_boundingRect.isValid()) {
            d_boundingRect.setLeft(d_boundingRect.right() - timespan_);
        }
    }
    return d_boundingRect;
}

void CurveDataTime::append( const QPointF &point ) {
    if (keepHiddenPoints_|| !d_boundingRect.isValid() || point.x() >= d_boundingRect.left()) {
        d_samples += point;

        if (d_boundingRect.isValid()) {
            if (point.x() > d_boundingRect.right()) {
                d_boundingRect.setRight(point.x());
                d_boundingRect.setLeft(d_boundingRect.right() - timespan_);

                if (!keepHiddenPoints_) {
                    // delete old points which are not inside bounding rectangle anymore
                    // We assume points to be chronologically ordered.
                    QVector<QPointF>::Iterator iter = d_samples.begin();
                    while(iter != d_samples.end()) {
                        if (d_boundingRect.contains(*iter)) break;
                        iter++;
                    }
                    d_samples.erase(d_samples.begin(), iter);
                }
            }

            if (point.y() > d_boundingRect.bottom()) {
                d_boundingRect.setBottom(point.y());
            } else if (point.y() < d_boundingRect.top()) {
                d_boundingRect.setTop(point.y());
            }
        }
    }
}



// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// CurveDataTimeFixedY
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
QRectF CurveDataTimeFixedY::boundingRect() const {
    if (!d_boundingRect.isValid()) {
        d_boundingRect = CurveDataBase::boundingRect();
        if (d_boundingRect.isValid()) {
            d_boundingRect.setLeft(d_boundingRect.right() - timespan_);
            d_boundingRect.setBottom(top);
            d_boundingRect.setTop(bottom);
        }
    }
    return d_boundingRect;
}

void CurveDataTimeFixedY::append( const QPointF &point ) {
    if (keepHiddenPoints_|| !d_boundingRect.isValid() || (point.x() >= d_boundingRect.left() && point.y() >= bottom && point.y() <= top)) {
        d_samples += point;

        if (d_boundingRect.isValid()) {
            if (point.x() > d_boundingRect.right()) {
                d_boundingRect.setRight(point.x());
                d_boundingRect.setLeft(d_boundingRect.right() - timespan_);

                if (!keepHiddenPoints_) {
                    // delete old points which are not inside bounding rectangle anymore
                    // We assume points to be chronologically ordered.
                    QVector<QPointF>::Iterator iter = d_samples.begin();
                    while(iter != d_samples.end()) {
                        if (d_boundingRect.contains(*iter)) break;
                        iter++;
                    }
                    d_samples.erase(d_samples.begin(), iter);
                }
            }
        }
    }
}



// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// CurveDataFixedXFixedY
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
CurveDataFixedXFixedY::CurveDataFixedXFixedY(qreal x, qreal width, qreal y, qreal height, bool keepHiddenPoints) :
    CurveDataBase(keepHiddenPoints)
{
    d_boundingRect = QRectF(QPointF(x, y), QSizeF(width, height));
}

QRectF CurveDataFixedXFixedY::boundingRect() const {
    return d_boundingRect;
}
void CurveDataFixedXFixedY::append( const QPointF &point ) {
    if (keepHiddenPoints_|| (
                point.y() >= d_boundingRect.top() &&
                point.y() <= d_boundingRect.bottom() &&
                point.x() >= d_boundingRect.left() &&
                point.x() <= d_boundingRect.right())) {
        d_samples += point;
    }
}
