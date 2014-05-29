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
#include <qwt_legend_label.h>
#include <qwt_scale_widget.h>
#include <libs/checkactionext.h>
#include <QMenu>
#include <qwt_symbol.h>
#include <QTableWidget>
#include "hoverableqwtlegend.h"
#include "canvaspicker.h"
#include <QComboBox>
#include <QVBoxLayout>
#include <QFont>
#include <QHeaderView>
#include <qwt_plot_marker.h>



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
    SplitterExt("genericDatagraphidentifier", parent), curvesWithRightYAxis(0)
{
    setChildrenCollapsible(false);

    plot = new QwtPlot(title);

    dataTableChannelList = new QComboBox;
    dataTable = new QTableWidget;
    dataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    dataTable->setSelectionMode(QAbstractItemView::SingleSelection);
    dataTable->setFocusPolicy(Qt::NoFocus);
    QFont font;
    font.setStyleHint(QFont::System);
    font.setPointSize(8);
    dataTable->setFont(font);
    dataTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    dataTable->verticalHeader()->hide();
    dataTable->setColumnCount(2);
    dataTable->setHorizontalHeaderLabels(QStringList{"x", "y"});

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(dataTableChannelList);
    layout->addWidget(dataTable);

    QWidget* containerWidget = new QWidget;
    containerWidget->setLayout(layout);

    addWidget(plot);
    addWidget(containerWidget);
    restoreState();
    widget(1)->hide();


    panner = new QwtPlotPanner( plot->canvas() );
    zoomer = new QwtPlotZoomer(plot->canvas());

    picker = new CanvasPicker( plot );
    connect(picker, SIGNAL(dataPointSuggested(int)), this, SLOT(showEntryInDatatable(int)));
    connect(picker, SIGNAL(nextPlotCurveSuggested()), this, SLOT(selectNextPlotCurve()));
    connect(picker, SIGNAL(previuosPlotCurveSuggested()), this, SLOT(selectPreviousPlotCurve()));
    connect(dataTable, SIGNAL(currentCellChanged(int,int,int,int)), picker, SLOT(selectPlotPoint(int)));


    // zoom in/out with the wheel
    QwtPlotMagnifier* magnifier = new QwtPlotMagnifier( plot->canvas() );
    magnifier->setWheelFactor(1.2);
    magnifier->setMouseButton(Qt::NoButton);

    HoverableQwtLegend *legend = new HoverableQwtLegend;
    plot->insertLegend(legend, QwtPlot::BottomLegend);
    legend->setDefaultItemMode( QwtLegendData::Checkable );
    connect(legend, SIGNAL(enter(const QVariant&)), this, SLOT(onHighlightCurve(const QVariant&)));
    connect(legend, SIGNAL(leave(const QVariant&)), this, SLOT(onUnhighlightCurve(const QVariant)));
    connect(legend, SIGNAL(checked(const QVariant &, bool, int)), SLOT(legendChecked(const QVariant &, bool)));
    connect(legend, SIGNAL(mouseMiddleClicked(QVariant)), this, SLOT(legendMouseMiddleClicked(QVariant)));

    QwtPlotPicker* d_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, plot->canvas());
//    d_picker->setStateMachine(new QwtPickerDragPointMachine());
    d_picker->setRubberBandPen(QColor(Qt::lightGray));
    d_picker->setRubberBand(QwtPicker::CrossRubberBand);
    d_picker->setTrackerPen(QColor(Qt::black));

    QPalette p = plot->axisWidget(QwtPlot::yRight)->palette();
    p.setColor(QPalette::Text, Qt::blue);
    plot->axisWidget(QwtPlot::yRight)->setPalette(p);

    // canvas
    static_cast<QwtPlotCanvas*>(plot->canvas())->setLineWidth(0);


    QPalette canvasPalette( Qt::white );
    canvasPalette.setColor( QPalette::Foreground, QColor( 133, 190, 232 ) );
    plot->canvas()->setPalette( canvasPalette );

    QAction* separator_action2 = new QAction(this);
    separator_action2->setSeparator(true);
    addAction(separator_action2);

    show_datatable_action = new QAction("Show data table", this);
    show_datatable_action->setIcon(QIcon::fromTheme("view-form-table", QIcon(":/images/view-form-table.png")));
    addAction(show_datatable_action);
    connect(show_datatable_action, SIGNAL(triggered(bool)), this, SLOT(showDataTable(bool)));
    show_datatable_action->setCheckable(true);


    QAction* zoom_fit_action = new QAction("Apply automatic zoom", this);
    zoom_fit_action->setIcon(QIcon::fromTheme("zoom-original", QIcon(":/images/zoom-original.png")));
    addAction(zoom_fit_action);
    connect(zoom_fit_action, SIGNAL(triggered()), this, SLOT(doAutoZoom()));

    QActionGroup* style_group = new QActionGroup(this);

    CheckActionExt* style_symbols = new CheckActionExt("DatagraphStyleKreuze", "Kreuze", false, this);
    style_symbols->setActionGroup(style_group);
    styleMapper.setMapping(style_symbols, static_cast<int>(Style::symbols));
    connect(style_symbols, SIGNAL(triggered()), &styleMapper, SLOT(map()));

    CheckActionExt* style_stepped = new CheckActionExt("DatagraphStyleGestuft", "Linie", false, this);
    style_stepped->setActionGroup(style_group);
    styleMapper.setMapping(style_stepped, static_cast<int>(Style::stepped_line));
    connect(style_stepped, SIGNAL(triggered()), &styleMapper, SLOT(map()));

    CheckActionExt* style_stepped_line_and_symbols = new CheckActionExt("DatagraphStyleSteppedLinieUndSymbol", "Linie + Kreuze", false, this);
    style_stepped_line_and_symbols->setActionGroup(style_group);
    styleMapper.setMapping(style_stepped_line_and_symbols, static_cast<int>(Style::stepped_line_and_symbols));
    connect(style_stepped_line_and_symbols, SIGNAL(triggered()), &styleMapper, SLOT(map()));

    CheckActionExt* style_interpolated = new CheckActionExt("DatagraphStyleInterpoliert", "Linie, interpoliert", false, this);
    style_interpolated->setActionGroup(style_group);
    styleMapper.setMapping(style_interpolated, static_cast<int>(Style::interpolated_line));
    connect(style_interpolated, SIGNAL(triggered()), &styleMapper, SLOT(map()));

    CheckActionExt* style_interpolated_line_and_symbols = new CheckActionExt("DatagraphStyleInterpoliertLinieUndSymbol", "Linie, interpoliert + Kreuze", true, this);
    style_interpolated_line_and_symbols->setActionGroup(style_group);
    styleMapper.setMapping(style_interpolated_line_and_symbols, static_cast<int>(Style::interpolated_line_and_symbols));
    connect(style_interpolated_line_and_symbols, SIGNAL(triggered()), &styleMapper, SLOT(map()));

    QAction* style_parent_action = new QAction("Curve style", this);
    style_parent_action->setMenu(new QMenu);
    style_parent_action->menu()->addActions(style_group->actions());
    connect(&styleMapper, SIGNAL(mapped(int)), this, SLOT(setCurveStyle(int)));

    for (QAction* action : style_group->actions()) {
        if (action->isChecked()) {
            action->trigger();
            break;
        }
    }

    addAction(style_parent_action);

    QActionGroup* zoom_group = new QActionGroup(this);

    CheckActionExt* zoom_box_zoom_action = new CheckActionExt("Datagraphzoom_box_zoom_action", "Activate box zoom", false, this);
    zoom_box_zoom_action->setActionGroup(zoom_group);
    zoom_box_zoom_action->setIcon(QIcon::fromTheme("zoom-select", QIcon(":/images/zoom-select.png")));
    connect(zoom_box_zoom_action, SIGNAL(triggered()), &userInputModeMapper, SLOT(map()));
    userInputModeMapper.setMapping(zoom_box_zoom_action, 0);

    CheckActionExt* zoom_drag_action = new CheckActionExt("Datagraphzoom_drag_action", "Activate graph panner", true, this);
    zoom_drag_action->setActionGroup(zoom_group);
    zoom_drag_action->setIcon(QIcon::fromTheme("transform-move", QIcon(":/images/transform-move.png")));
    connect(zoom_drag_action, SIGNAL(triggered()), &userInputModeMapper, SLOT(map()));
    userInputModeMapper.setMapping(zoom_drag_action, 1);

    QAction* canvas_pick_action = new QAction("Activate plot picker", this);
    canvas_pick_action->setActionGroup(zoom_group);
    canvas_pick_action->setCheckable(true);
    canvas_pick_action->setIcon(QIcon::fromTheme("edit-node", QIcon(":/images/edit-node.png")));
    connect(canvas_pick_action, SIGNAL(triggered()), &userInputModeMapper, SLOT(map()));
    userInputModeMapper.setMapping(canvas_pick_action, 2);

    QAction* separator_action = new QAction(this);
    separator_action->setSeparator(true);
    addAction(separator_action);

    addActions(zoom_group->actions());
    connect(&userInputModeMapper, SIGNAL(mapped(int)), this, SLOT(setUserInputMode(int)));

    for (QAction* action : zoom_group->actions()) {
        if (action->isChecked()) {
            action->trigger();
            break;
        }
    }


    separator_action = new QAction(this);
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

    QAction* export_action = new QAction("Export Graph", this);
    export_action->setIcon(QIcon::fromTheme("document-export", QIcon(":/images/document-export.png")));
    addAction(export_action);
    connect(export_action, SIGNAL(triggered()), this, SLOT(exportOutput()));

    connect(&refreshTimer, SIGNAL(timeout()), this, SLOT(execReplot()));
}

DataGraph::~DataGraph() {
}

void DataGraph::execReplot(void) {
    plot->replot();
    refreshTimer.stop();
}

void DataGraph::requestReplot(void) {
    if (!refreshTimer.isActive()) {
        refreshTimer.start(100);
    }
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
    curve->attach(plot);
    curve->setData(curveData);

    channels.append(curve);
    updateCurveColors();
    applyCurveStyleToCurve(curve);
    showCurve(curve, true);

    dataTableChannelList->addItem(title);
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
        if (channels.at(index)->yAxis() == QwtPlot::yRight) {
            --curvesWithRightYAxis;
            updateRightAxis();
        }

        channels.at(index)->detach();
        delete channels.at(index);
        channels.removeAt(index);

        dataTableChannelList->removeItem(index);
    }
    updateCurveColors();
}

void DataGraph::clear() {
    for (auto iter : channels) {
        iter->detach();
        delete iter;
    }
    channels.clear();

    curvesWithRightYAxis = 0;
    updateRightAxis();

    dataTableChannelList->clear();
    dataTable->clearContents();

    channelGroups.clear();

    doAutoZoom();
}


void DataGraph::setUserInputMode(int index) {
    switch (index) {
    case 0:
        zoomer->setEnabled(true);
        panner->setEnabled(false);
        picker->setEnabled(false);
        zoomer->setZoomBase();
        break;

    case 1:
        zoomer->setEnabled(false);
        panner->setEnabled(true);
        picker->setEnabled(false);
        break;

    case 2:
        zoomer->setEnabled(false);
        panner->setEnabled(false);
        picker->setEnabled(true);
        showDataTable(true);
        show_datatable_action->setChecked(true);
        break;
    }
}

void DataGraph::onHighlightCurve(const QVariant &itemInfo) {
    QwtPlotItem *plotItem = plot->infoToItem(itemInfo);
    if (plotItem) {
        QwtPlotCurve *curve = dynamic_cast<QwtPlotCurve *>(plotItem);

        if (curve) {
            QPen pen = curve->pen();
            pen.setWidth(pen.width() * 5);
            curve->setPen(pen);

            QwtLegend *lgd = qobject_cast<QwtLegend *>(plot->legend());
            QWidget * legendWidget = lgd->legendWidget(itemInfo);
            if (legendWidget) {
                HoverableQwtLegendLabel *legendLabel = qobject_cast<HoverableQwtLegendLabel *>(legendWidget);
                if (legendLabel) {
                    legendLabel->highlight();
                    requestReplot();
                }
            }
        }
    }
}

void DataGraph::onUnhighlightCurve(const QVariant &itemInfo) {
    QwtPlotItem *plotItem = plot->infoToItem(itemInfo);
    if (plotItem) {
        QwtPlotCurve *curve = dynamic_cast<QwtPlotCurve *>(plotItem);

        if (curve) {
            QPen pen = curve->pen();
            pen.setWidth(pen.width() / 5);
            curve->setPen(pen);

            QwtLegend *lgd = qobject_cast<QwtLegend *>(plot->legend());
            QWidget * legendWidget = lgd->legendWidget(itemInfo);
            if (legendWidget) {
                HoverableQwtLegendLabel *legendLabel = qobject_cast<HoverableQwtLegendLabel *>(legendWidget);
                if (legendLabel) {
                    legendLabel->unhighlight();
                    requestReplot();
                }
            }
        }
    }
}

void DataGraph::addData(int channel, QPointF data) {
    if (channel < channels.size()) {
        CurveDataBase *curvedata = static_cast<CurveData *>( channels.at(channel)->data() );
        curvedata->append(data);

        show_datatable_action->setChecked(false);
        showDataTable(false);

        requestReplot();
    } else {
        (void) data;
    }
}

void DataGraph::addVerticalMarker(float time) {
    QwtPlotMarker* marker = new QwtPlotMarker;
    marker->setLineStyle(QwtPlotMarker::VLine);
    marker->attach(plot);
    marker->setXValue(time);

    QPen pen;
    pen.setStyle(Qt::DotLine);
    pen.setColor(Qt::black);
    pen.setWidth(1);
    marker->setLinePen(pen);

    vMarkers.append(marker);
    requestReplot();
}

void DataGraph::focusVerticalMarker(unsigned index) {

}

void DataGraph::doAutoZoom(void) {
    for (QwtPlotCurve* channel : channels) {
        CurveDataBase *curvedata = static_cast<CurveData *>( channel->data() );
        curvedata->resetBoundingRect();
    }
    plot->setAxisAutoScale(QwtPlot::xBottom, true);
    plot->setAxisAutoScale(QwtPlot::yLeft, true);
    plot->setAxisAutoScale(QwtPlot::yRight, true);

    requestReplot();

    zoomer->setZoomBase();
}

void DataGraph::legendChecked(const QVariant &itemInfo, bool on) {
    QwtPlotItem *plotItem = plot->infoToItem(itemInfo);
    if (plotItem) {
        showCurve(plotItem, on);
    }
}

void DataGraph::showCurve(QwtPlotItem *item, bool on) {
    showCurve(item, on, true);
}

void DataGraph::showCurve(QwtPlotItem *item, bool on, bool visible) {
    if (visible) {
        item->setVisible(on);
    } else {
        item->setVisible(false);
    }

    QwtLegend *lgd = qobject_cast<QwtLegend *>(plot->legend());
    QList<QWidget *> legendWidgets = lgd->legendWidgets(plot->itemToInfo(item));

    if (legendWidgets.size() == 1) {
        QwtLegendLabel *legendLabel = qobject_cast<QwtLegendLabel *>(legendWidgets[0]);

        if (legendLabel) {
            if (visible) {
                legendLabel->setChecked(on);
                legendLabel->setVisible(true);
            } else {
                legendLabel->setVisible(false);
            }
        }
    }

    requestReplot();
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

void DataGraph::setCurveStyle(int styleIndex) {
    selectedStyle = styleIndex;

    for (QwtPlotCurve* curve : channels) {
        applyCurveStyleToCurve(curve);
    }
}

void DataGraph::applyCurveStyleToCurve(QwtPlotCurve* curve) {
    switch (static_cast<DataGraph::Style>(selectedStyle)) {
    case Style::symbols:
        curve->setStyle( QwtPlotCurve::Dots );
        curve->setSymbol(new QwtSymbol( QwtSymbol::XCross, Qt::NoBrush, curve->pen(), QSize( 8, 8 ) ));
        curve->setRenderHint(QwtPlotItem::RenderHint::RenderAntialiased, false);
        break;

    case Style::interpolated_line:
        curve->setStyle( QwtPlotCurve::Lines );
        curve->setSymbol(nullptr);
        curve->setRenderHint(QwtPlotItem::RenderHint::RenderAntialiased, true);
        break;

    case Style::stepped_line:
        curve->setStyle( QwtPlotCurve::Steps );
        curve->setSymbol(nullptr);
        curve->setCurveAttribute( QwtPlotCurve::Inverted, true);
        curve->setRenderHint(QwtPlotItem::RenderHint::RenderAntialiased, false);
        break;

    case Style::interpolated_line_and_symbols:
        curve->setStyle( QwtPlotCurve::Lines );
        curve->setSymbol(new QwtSymbol( QwtSymbol::Cross, Qt::NoBrush, curve->pen(), QSize( 8, 8 ) ));
        curve->setRenderHint(QwtPlotItem::RenderHint::RenderAntialiased, true);
        break;

    case Style::stepped_line_and_symbols:
        curve->setStyle( QwtPlotCurve::Steps );
        curve->setSymbol(new QwtSymbol( QwtSymbol::XCross, Qt::NoBrush, curve->pen(), QSize( 8, 8 ) ));
        curve->setCurveAttribute( QwtPlotCurve::Inverted, true);
        curve->setRenderHint(QwtPlotItem::RenderHint::RenderAntialiased, false);
        break;
    }

    requestReplot();
}

void DataGraph::legendMouseMiddleClicked(const QVariant &itemInfo) {
    QwtPlotItem *plotItem = plot->infoToItem(itemInfo);

    if (plotItem) {
        QwtPlotCurve *curve = dynamic_cast<QwtPlotCurve *>(plotItem);

        if (curve) {
            if (curve->yAxis() == QwtPlot::yLeft) {
                curve->setYAxis(QwtPlot::yRight);
                ++curvesWithRightYAxis;
            } else {
                curve->setYAxis(QwtPlot::yLeft);
                --curvesWithRightYAxis;
            }

            QwtLegend *lgd = qobject_cast<QwtLegend *>(plot->legend());
            QWidget * legendWidget = lgd->legendWidget(itemInfo);
            if (legendWidget) {
                HoverableQwtLegendLabel *legendLabel = qobject_cast<HoverableQwtLegendLabel *>(legendWidget);
                if (legendLabel) {
                    if (curve->yAxis() == QwtPlot::yLeft) {
                        legendLabel->setLeftSide();
                    } else {
                        legendLabel->setRightSide();
                    }
                }
            }

            updateRightAxis();
            requestReplot();
        }
    }
}


void DataGraph::updateCurveColors() {
    ColorMapDiscrete2 colormap(channels.size());

    for (int i = 0; i < channels.size(); ++i) {
        channels.at(i)->setPen(QPen(colormap.getColor(i)));
    }
}


bool DataGraph::exportOutput(QString fileName) {
    if ( !fileName.isEmpty() ) {
        if (fileName.endsWith(".csv")) {
            QFile data(fileName);
            if (data.open(QFile::WriteOnly | QFile::Truncate)) {
                QTextStream out(&data);



                out << "Result: " << qSetFieldWidth(10) << left << 3.14 << 2.7;
                // writes "Result: 3.14      2.7       "
                return true;
            } else {
                return false;
            }
        } else {

            QwtPlotRenderer renderer;

            // flags to make the document look like the widget
            renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, true);
            renderer.setLayoutFlag(QwtPlotRenderer::DefaultLayout, true);

            renderer.renderDocument(plot, fileName, QSizeF(300, 200), 85);

            return true;
        }
    } else {
        return false;
    }
}

bool DataGraph::exportOutput(void) {
    QString fileName = "plot.png";

    QStringList filter;
    filter += "CSV-data (*.csv)";
    filter += "PNG-Images (*.png)";
    filter += "JPEG-Images (*.jpg)";
    filter += "PDF Documents (*.pdf)";
#ifndef QWT_NO_SVG
    filter += "SVG Documents (*.svg)";
#endif
    filter += "Postscript Documents (*.ps)";

    fileName = QFileDialog::getSaveFileName(this, "Export File Name", fileName,
        filter.join(";;"), NULL);

    return exportOutput(fileName);
}

void DataGraph::updateRightAxis(void) {
    if (curvesWithRightYAxis == 0) {
        plot->enableAxis(QwtPlot::yRight, false);
    } else {
        plot->enableAxis(QwtPlot::yRight, true);
    }
    if (curvesWithRightYAxis == channels.size() && channels.size()) {
        plot->enableAxis(QwtPlot::yLeft, false);
    } else {
        plot->enableAxis(QwtPlot::yLeft, true);
    }
}

void DataGraph::showDataTable(bool show) {
    if (show) {
        widget(1)->show();
        connect(dataTableChannelList, SIGNAL(currentIndexChanged(int)), this, SLOT(generateDataTableForChannel(int)));
        generateDataTableForChannel(dataTableChannelList->currentIndex());
    } else {
        widget(1)->hide();
        disconnect(dataTableChannelList, SIGNAL(currentIndexChanged(int)), this, SLOT(generateDataTableForChannel(int)));
    }
}

void DataGraph::generateDataTableForChannel(int index) {
    if (index >= 0 && index < static_cast<int>(channels.at(index)->data()->size())) {
        int oldIndex = dataTable->currentRow();

        dataTable->clearContents();
        dataTable->setRowCount(channels.at(index)->data()->size());

        CurveDataBase *curvedata = static_cast<CurveData *>( channels.at(index)->data() );

        for (unsigned i = 0; i < channels.at(index)->data()->size(); ++i) {
            addEntryToDataTable(curvedata->sample(i), i);
        }
        dataTable->resizeRowsToContents();

        picker->selectPlotCurve(channels.at(index));

        showEntryInDatatable(oldIndex);
    }
}

void DataGraph::addEntryToDataTable(QPointF data, int row) {
    if (row == -1) {
        row = dataTable->rowCount();
        dataTable->setRowCount(row + 1);
    }

    QTableWidgetItem *newItem = new QTableWidgetItem(QString("%1").arg(data.x()));
    dataTable->setItem(row, 0, newItem);
    newItem = new QTableWidgetItem(QString("%1").arg(data.y()));
    dataTable->setItem(row, 1, newItem);
}

void DataGraph::showEntryInDatatable(int index) {
    dataTable->scrollToItem(dataTable->item(index, 0), QAbstractItemView::PositionAtCenter);
    dataTable->selectRow(index);
}

void DataGraph::selectNextPlotCurve(void) {
    if (dataTableChannelList->count()) {
        if (dataTableChannelList->currentIndex() == -1 || dataTableChannelList->currentIndex() == dataTableChannelList->count()  - 1) {
            dataTableChannelList->setCurrentIndex(0);
        } else {
            dataTableChannelList->setCurrentIndex(dataTableChannelList->currentIndex() + 1);
        }
    }
    generateDataTableForChannel(dataTableChannelList->currentIndex());
}

void DataGraph::selectPreviousPlotCurve(void) {
    if (dataTableChannelList->count()) {
        if (dataTableChannelList->currentIndex() == -1 || dataTableChannelList->currentIndex() == 0) {
            dataTableChannelList->setCurrentIndex(dataTableChannelList->count() - 1);
        } else {
            dataTableChannelList->setCurrentIndex(dataTableChannelList->currentIndex() - 1);
        }
    }
    generateDataTableForChannel(dataTableChannelList->currentIndex());
}


void DataGraph::addChannelGroup(const QList<int>& channelGroup) {
    channelGroups.append(channelGroup);
}

void DataGraph::applyChannelGrouping(int index) {
    for (int i = 0; i < channels.size(); ++i) {
        if (channelGroups.at(index).contains(i)) {
            showCurve(channels.at(i), true);
        } else {
            showCurve(channels.at(i), false, false);
        }
    }
}

void DataGraph::resetChannelGrouping(void) {
    showAllCurves();
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
