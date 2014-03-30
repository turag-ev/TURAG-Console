#include "datagraph.h"
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_zoomer.h>
#include <qwt_interval.h>
#include <qwt_legend_label.h>
#include <QPen>
#include <QColor>
#include <QImageWriter>
#include <QList>
#include <QStringList>
#include <QByteArray>
#include <QFileDialog>
#include <QSignalMapper>
#include "colormap.h"
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <QDebug>
#include <QVector>
#include <QAction>
#include <QActionGroup>
#include <QSettings>
#include <qwt_legend_label.h>

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

    HoverableQwtLegend *legend = new HoverableQwtLegend;
    insertLegend(legend, QwtPlot::BottomLegend);
    legend->setDefaultItemMode( QwtLegendData::Checkable );
    connect(legend, SIGNAL(enter(const QVariant&)), this, SLOT(onHighlightCurve(const QVariant&)));
    connect(legend, SIGNAL(leave(const QVariant&)), this, SLOT(onUnhighlightCurve(const QVariant)));
    connect(legend, SIGNAL(checked(const QVariant &, bool, int)), SLOT(legendChecked(const QVariant &, bool)));

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

    QAction* separator_action2 = new QAction(this);
    separator_action2->setSeparator(true);
    addAction(separator_action2);

    QAction* zoom_fit_action = new QAction("Apply automatic zoom", this);
    addAction(zoom_fit_action);
    connect(zoom_fit_action, SIGNAL(triggered()), this, SLOT(doAutoZoom()));

    QActionGroup* zoom_group = new QActionGroup(this);

    zoom_box_zoom_action = new QAction("Activate box zoom", this);
    zoom_box_zoom_action->setActionGroup(zoom_group);
    connect(zoom_box_zoom_action, SIGNAL(triggered()), this, SLOT(setZoomer()));
    zoom_box_zoom_action->setCheckable(true);

    zoom_drag_action = new QAction("Activate graph panner", this);
    connect(zoom_drag_action, SIGNAL(triggered()), this, SLOT(setPanner()));
    zoom_drag_action->setActionGroup(zoom_group);
    zoom_drag_action->setCheckable(true);

    addActions(zoom_group->actions());

    QAction* separator_action = new QAction(this);
    separator_action->setSeparator(true);
    addAction(separator_action);

    QAction* hide_all_action = new QAction("Hide all graphs", this);
    addAction(hide_all_action);
    connect(hide_all_action, SIGNAL(triggered()), this, SLOT(hideAllCurves()));

    QAction* show_all_action = new QAction("Show all graphs", this);
    addAction(show_all_action);
    connect(show_all_action, SIGNAL(triggered()), this, SLOT(showAllCurves()));

    QAction* separator_action3 = new QAction(this);
    separator_action3->setSeparator(true);
    addAction(separator_action3);

    QAction* save_action = new QAction("Save Graph", this);
    addAction(save_action);
    connect(save_action, SIGNAL(triggered()), this, SLOT(saveOutput()));

    QSettings settings;
    settings.beginGroup(objectName());
    zoom_drag_action->setChecked(settings.value("zoom_drag_action", true).toBool());
    zoom_box_zoom_action->setChecked(!(settings.value("zoom_drag_action", true).toBool()));

    if (zoom_drag_action->isChecked()) {
        setPanner();
    } else {
        setZoomer();
    }
}

DataGraph::~DataGraph() {
    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("zoom_drag_action", zoom_drag_action->isChecked());
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
    curve->setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
    curve->attach(this);
    curve->setData(curveData);
    curve->setRenderHint(QwtPlotItem::RenderHint::RenderAntialiased, true);

    channels.append(curve);
    updateCurveColors();
    showCurve(curve, true);
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
   zoomer->setZoomBase();
}

void DataGraph::setPanner(void) {
    zoomer->setEnabled(false);
    panner->setEnabled(true);
}

void DataGraph::onHighlightCurve(const QVariant &itemInfo) {
    QwtPlotItem *plotItem = infoToItem(itemInfo);
    if (plotItem) {
        QwtPlotCurve *curve = dynamic_cast<QwtPlotCurve *>(plotItem);

        if (curve) {
            QPen pen = curve->pen();
            pen.setWidth(pen.width() * 5);
            curve->setPen(pen);

            QwtLegend *lgd = qobject_cast<QwtLegend *>(legend());
            QWidget * legendWidget = lgd->legendWidget(itemInfo);
            if (legendWidget) {
                HoverableQwtLegendLabel *legendLabel = qobject_cast<HoverableQwtLegendLabel *>(legendWidget);
                if (legendLabel) {
                    legendLabel->highlight();
                }
            }
            replot();
        }
    }
}

void DataGraph::onUnhighlightCurve(const QVariant &itemInfo) {
    QwtPlotItem *plotItem = infoToItem(itemInfo);
    if (plotItem) {
        QwtPlotCurve *curve = dynamic_cast<QwtPlotCurve *>(plotItem);

        if (curve) {
            QPen pen = curve->pen();
            pen.setWidth(pen.width() / 5);
            curve->setPen(pen);

            QwtLegend *lgd = qobject_cast<QwtLegend *>(legend());
            QWidget * legendWidget = lgd->legendWidget(itemInfo);
            if (legendWidget) {
                HoverableQwtLegendLabel *legendLabel = qobject_cast<HoverableQwtLegendLabel *>(legendWidget);
                if (legendLabel) {
                    legendLabel->unhighlight();
                }
            }
            replot();
        }
    }
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

    zoomer->setZoomBase();
}

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

void DataGraph::showAllCurves(void) {
    for (QwtPlotCurve* curve : channels) {
        showCurve(curve, true);
    }
}

void DataGraph::hideAllCurves(void) {
    for (QwtPlotCurve* curve : channels) {
        showCurve(curve, false);
    }
}


void DataGraph::updateCurveColors() {
    ColorMapDiscrete2 colormap(channels.size());

    for (int i = 0; i < channels.size(); ++i) {
        channels.at(i)->setPen(QPen(colormap.getColor(i)));
    }
}


bool DataGraph::saveOutput(QString fileName) {
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

bool DataGraph::saveOutput(void) {
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

    return saveOutput(fileName);
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
