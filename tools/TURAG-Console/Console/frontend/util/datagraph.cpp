#include "datagraph.h"
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_plot_canvas.h>
#include <qwt_series_data.h>
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

#if QWT_VERSION < 0x060100
# include <qwt_legend_item.h>
#else
# include <qwt_legend_label.h>
#endif

class CurveData: public QwtArraySeriesData<QPointF>
{
public:
    CurveData() { }

    virtual QRectF boundingRect() const {
        if (!d_boundingRect.isValid()) {
            d_boundingRect = qwtBoundingRect( *this );
        }
        return d_boundingRect;
    }

    inline void append( const QPointF &point ) {
        d_samples += point;

        if (d_boundingRect.isValid() && !d_boundingRect.contains(point)) {
            if (point.x() > d_boundingRect.right()) {
                d_boundingRect.setRight(point.x());
            } else if (point.x() < d_boundingRect.left()) {
                d_boundingRect.setLeft(point.x());
            }
            if (point.y() > d_boundingRect.top()) {
                d_boundingRect.setTop(point.y());
            } else if (point.y() < d_boundingRect.bottom()) {
                d_boundingRect.setBottom(point.y());
            }
        }
    }
};



// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// class DataGraph
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

DataGraph::DataGraph(QWidget *parent) :
    QwtPlot(parent)
{
    // panning with the left mouse button
//    (void) new QwtPlotPanner( canvas() );

    // zoom in/out with the wheel
//    (void) new QwtPlotMagnifier( canvas() );

//    zoomer = new QwtPlotZoomer(canvas());

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
    d_picker->setStateMachine(new QwtPickerDragPointMachine());
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


void DataGraph::addChannel(QString title) {
    QwtPlotCurve *curve = new QwtPlotCurve(title);
    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve->setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
    curve->attach(this);
    curve->setData(new CurveData);

    channels.append(curve);
    updateCurveColors();
    showCurve(curve, true);

    //    curve->setPen(QPen(Qt::red));
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


void DataGraph::addData(int channel, QPointF data) {
    if (channel < channels.size()) {
        CurveData *curvedata = static_cast<CurveData *>( channels.at(channel)->data() );
        curvedata->append(data);
    } else {
        (void) data;
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
