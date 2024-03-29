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
#include <libs/iconmanager.h>
#include "plotmagnifier.h"



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
	QSplitter(parent), curvesWithRightYAxis(0), dataTable(nullptr), dataTableChannelList(nullptr),
	updateCurveColorsRequested(false), appliedChannelGrouping(-1)
{
    setChildrenCollapsible(false);
	setMinimumHeight(300);

    plot = new QwtPlot(title);
    addWidget(plot);

	setContextMenuPolicy(Qt::ActionsContextMenu);

    panner = new QwtPlotPanner( plot->canvas() );
	panner->setMouseButton(Qt::MiddleButton);

    zoomer = new QwtPlotZoomer(plot->canvas());
	zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::MidButton, Qt::ControlModifier);
	zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::MidButton, Qt::AltModifier);

    picker = new CanvasPicker( plot );
    connect(picker, SIGNAL(dataPointSuggested(int)), this, SLOT(showEntryInDatatable(int)));
    connect(picker, SIGNAL(nextPlotCurveSuggested()), this, SLOT(selectNextPlotCurve()));
    connect(picker, SIGNAL(previuosPlotCurveSuggested()), this, SLOT(selectPreviousPlotCurve()));


    // zoom in/out with the wheel
	PlotMagnifier* magnifier = new PlotMagnifier( plot->canvas() );
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
    show_datatable_action->setIcon(IconManager::get("view-form-table"));
    addAction(show_datatable_action);
    connect(show_datatable_action, SIGNAL(triggered(bool)), this, SLOT(showDataTable(bool)));
    show_datatable_action->setCheckable(true);


    QAction* zoom_fit_action = new QAction("Apply automatic zoom", this);
    zoom_fit_action->setIcon(IconManager::get("zoom-original"));
    addAction(zoom_fit_action);
    connect(zoom_fit_action, SIGNAL(triggered()), this, SLOT(doAutoZoom()));

    QActionGroup* style_group = new QActionGroup(this);

	CheckActionExt* style_symbols = new CheckActionExt("DatagraphStyleKreuze", "Kreuze", false, this, false);
    style_symbols->setActionGroup(style_group);
    styleMapper.setMapping(style_symbols, static_cast<int>(Style::symbols));
    connect(style_symbols, SIGNAL(triggered()), &styleMapper, SLOT(map()));

	CheckActionExt* style_stepped = new CheckActionExt("DatagraphStyleGestuft", "Linie", false, this, false);
    style_stepped->setActionGroup(style_group);
    styleMapper.setMapping(style_stepped, static_cast<int>(Style::stepped_line));
    connect(style_stepped, SIGNAL(triggered()), &styleMapper, SLOT(map()));

	CheckActionExt* style_stepped_line_and_symbols = new CheckActionExt("DatagraphStyleSteppedLinieUndSymbol", "Linie + Kreuze", false, this, false);
    style_stepped_line_and_symbols->setActionGroup(style_group);
    styleMapper.setMapping(style_stepped_line_and_symbols, static_cast<int>(Style::stepped_line_and_symbols));
    connect(style_stepped_line_and_symbols, SIGNAL(triggered()), &styleMapper, SLOT(map()));

	CheckActionExt* style_interpolated = new CheckActionExt("DatagraphStyleInterpoliert", "Linie, interpoliert", true, this, false);
    style_interpolated->setActionGroup(style_group);
    styleMapper.setMapping(style_interpolated, static_cast<int>(Style::interpolated_line));
    connect(style_interpolated, SIGNAL(triggered()), &styleMapper, SLOT(map()));

	CheckActionExt* style_interpolated_line_and_symbols = new CheckActionExt("DatagraphStyleInterpoliertLinieUndSymbol", "Linie, interpoliert + Kreuze", false, this, false);
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

	CheckActionExt* zoom_box_zoom_action = new CheckActionExt("Datagraphzoom_box_zoom_action", "Activate box zoom", true, this, false);
    zoom_box_zoom_action->setActionGroup(zoom_group);
    zoom_box_zoom_action->setIcon(IconManager::get("zoom-select"));
    connect(zoom_box_zoom_action, SIGNAL(triggered()), &userInputModeMapper, SLOT(map()));
    userInputModeMapper.setMapping(zoom_box_zoom_action, 0);

//	CheckActionExt* zoom_drag_action = new CheckActionExt("Datagraphzoom_drag_action", "Activate graph panner", false, this, false);
//    zoom_drag_action->setActionGroup(zoom_group);
//    zoom_drag_action->setIcon(IconManager::get("transform-move"));
//    connect(zoom_drag_action, SIGNAL(triggered()), &userInputModeMapper, SLOT(map()));
//    userInputModeMapper.setMapping(zoom_drag_action, 1);

    QAction* canvas_pick_action = new QAction("Activate plot picker", this);
    canvas_pick_action->setActionGroup(zoom_group);
    canvas_pick_action->setCheckable(true);
    canvas_pick_action->setIcon(IconManager::get("edit-node"));
    connect(canvas_pick_action, SIGNAL(triggered()), &userInputModeMapper, SLOT(map()));
	userInputModeMapper.setMapping(canvas_pick_action, 1);

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
    export_action->setIcon(IconManager::get("document-export"));
    addAction(export_action);
    connect(export_action, SIGNAL(triggered()), this, SLOT(exportOutput()));

    connect(&refreshTimer, SIGNAL(timeout()), this, SLOT(execReplot()));
}

DataGraph::~DataGraph() {
}

void DataGraph::copyMetadata(DataGraph& source) {
	setTitle(source.plot->title().text());

	for (QwtPlotCurve* channel : source.channels) {
		CurveDataBase* curveData = static_cast<CurveDataBase*>(channel->data());

		this->addChannelGeneric(channel->title().text(), curveData->createInstance());
	}

	for (QList<int> channelList : source.channelGroups) {
		channelGroups.append(channelList);
	}
}

void DataGraph::execReplot(void) {
	if (updateCurveColorsRequested) {
		ColorMapDiscrete2 colormap(channels.size());

		for (int i = 0; i < channels.size(); ++i) {
			channels.at(i)->setPen(QPen(colormap.getColor(i)));
		}
		updateCurveColorsRequested = false;
	}
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

	if (dataTableChannelList) {
		dataTableChannelList->addItem(title);
	}
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

		if (dataTableChannelList) {
			dataTableChannelList->removeItem(index);
		}
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

	if (dataTableChannelList) {
		dataTableChannelList->clear();
	}
	if (dataTable) {
		dataTable->clearContents();
	}

    channelGroups.clear();

    doAutoZoom();
}

void DataGraph::clearData(void) {
	for (QwtPlotCurve* channel : channels) {
		CurveDataBase* curveData = static_cast<CurveDataBase*>(channel->data());
		curveData->clear();
	}

	curvesWithRightYAxis = 0;
	updateRightAxis();

	if (dataTable) {
		dataTable->clearContents();
	}

	doAutoZoom();
}


void DataGraph::setUserInputMode(int index) {
    switch (index) {
    case 0:
        zoomer->setEnabled(true);
		panner->setEnabled(true);
        picker->setEnabled(false);
        zoomer->setZoomBase();
        break;

	case 1:
        zoomer->setEnabled(false);
		panner->setEnabled(true);
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

void DataGraph::addData(int channel, QPointF data_) {
    if (channel < channels.size()) {
        CurveDataBase *curvedata = static_cast<CurveData *>( channels.at(channel)->data() );
		curvedata->append(data_);

        show_datatable_action->setChecked(false);
		showDataTable(false);

        requestReplot();
    } else {
		(void) data_;
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
		CurveDataBase *curvedata = static_cast<CurveDataBase *>( channel->data() );
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
	applyChannelGrouping(appliedChannelGrouping);
}

void DataGraph::hideAllCurves(void) {
	applyChannelGrouping(appliedChannelGrouping, false);
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
	updateCurveColorsRequested = true;
	requestReplot();
}


bool DataGraph::exportOutput(QString fileName) {
    if ( !fileName.isEmpty() ) {
        if (fileName.endsWith(".csv")) {
			QFile data_(fileName);
			if (data_.open(QFile::WriteOnly | QFile::Truncate)) {
                static constexpr char FIELD_DELIM[] = ",";
                static constexpr char ROW_DELIM[] = "\n";
				QTextStream out(&data_);
                out << "\"title\"" << FIELD_DELIM << "\"x\"" << FIELD_DELIM << "\"y\"" << ROW_DELIM;
                for (QwtPlotCurve* curve : channels) {
                    QString title = "\"" + curve->title().text().toHtmlEscaped() + "\"";
                    for (size_t i = 0; i < curve->data()->size(); i++) {
                        auto x = curve->data()->sample(i).x();
                        auto y = curve->data()->sample(i).y();
                        out << title << FIELD_DELIM << x << FIELD_DELIM << y << ROW_DELIM;
                    }
                }
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

	// Setting the parent causes trouble in windows.
    QWidget* parent = nullptr;
#ifndef Q_OS_WIN32
    parent = this;
#endif

    fileName = QFileDialog::getSaveFileName(parent, "Export File Name", fileName,
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
		if (dataTable == nullptr) {
			createDataTable();
		}
		if (widget(1)->isHidden()) {
			widget(1)->show();
			connect(dataTableChannelList, SIGNAL(currentIndexChanged(int)), this, SLOT(generateDataTableForChannel(int)));
			generateDataTableForChannel(dataTableChannelList->currentIndex());
		}
	} else if (!show && widget(1) && widget(1)->isVisible()) {
		widget(1)->hide();
		disconnect(dataTableChannelList, SIGNAL(currentIndexChanged(int)), this, SLOT(generateDataTableForChannel(int)));
	}
}

void DataGraph::createDataTable(void) {
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
	addWidget(containerWidget);
	containerWidget->hide();
	setStretchFactor(0, 2);

	connect(dataTable, SIGNAL(currentCellChanged(int,int,int,int)), picker, SLOT(selectPlotPoint(int)));

	for (QwtPlotCurve* channel : channels) {
		dataTableChannelList->addItem(channel->title().text());
	}
}

void DataGraph::generateDataTableForChannel(int index) {
	if (index >= 0 && index < channels.size()) {
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

void DataGraph::addEntryToDataTable(QPointF data_, int row) {
    if (row == -1) {
        row = dataTable->rowCount();
        dataTable->setRowCount(row + 1);
    }

	QTableWidgetItem *newItem = new QTableWidgetItem(QString("%1").arg(data_.x()));
    dataTable->setItem(row, 0, newItem);
	newItem = new QTableWidgetItem(QString("%1").arg(data_.y()));
    dataTable->setItem(row, 1, newItem);
}

void DataGraph::showEntryInDatatable(int index) {
	if (dataTable) {
		dataTable->scrollToItem(dataTable->item(index, 0), QAbstractItemView::PositionAtCenter);
		dataTable->selectRow(index);
	}
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

void DataGraph::applyChannelGrouping(int index, bool showCurves) {
    for (int i = 0; i < channels.size(); ++i) {
		if (index < 0 || index >= channelGroups.size() || channelGroups.at(index).contains(i)) {
			showCurve(channels.at(i), showCurves);
        } else {
            showCurve(channels.at(i), false, false);
        }
    }

	appliedChannelGrouping = index;
}

void DataGraph::resetChannelGrouping(void) {
	for (QwtPlotCurve* curve : channels) {
		showCurve(curve, true);
	}

	appliedChannelGrouping = -1;
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

    if (QWT_SERIES_SAMPLES.size()) {
        rect.setRight(QWT_SERIES_SAMPLES[0].x());
        rect.setLeft(QWT_SERIES_SAMPLES[0].x());
        rect.setTop(QWT_SERIES_SAMPLES[0].y());
        rect.setBottom(QWT_SERIES_SAMPLES[0].y());
    }

    for (int i = 1; i < QWT_SERIES_SAMPLES.size(); ++i) {
        const QPointF& point = QWT_SERIES_SAMPLES[i];
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
    if (!QWT_SERIES_BOUNDING_RECT.isValid()) {
        QWT_SERIES_BOUNDING_RECT = CurveDataBase::boundingRect();
    }
    return QWT_SERIES_BOUNDING_RECT;
}


void CurveData::append( const QPointF &point ) {
    QWT_SERIES_SAMPLES += point;

    if (QWT_SERIES_BOUNDING_RECT.isValid()) {
        if (point.x() > QWT_SERIES_BOUNDING_RECT.right()) {
            QWT_SERIES_BOUNDING_RECT.setRight(point.x());
        } else if (point.x() < QWT_SERIES_BOUNDING_RECT.left()) {
            QWT_SERIES_BOUNDING_RECT.setLeft(point.x());
        }
        if (point.y() > QWT_SERIES_BOUNDING_RECT.bottom()) {
            QWT_SERIES_BOUNDING_RECT.setBottom(point.y());
        } else if (point.y() < QWT_SERIES_BOUNDING_RECT.top()) {
            QWT_SERIES_BOUNDING_RECT.setTop(point.y());
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
    if (!QWT_SERIES_BOUNDING_RECT.isValid()) {
        QWT_SERIES_BOUNDING_RECT = CurveDataBase::boundingRect();
        QWT_SERIES_BOUNDING_RECT.setLeft(left);
        QWT_SERIES_BOUNDING_RECT.setRight(right);
    }
    return QWT_SERIES_BOUNDING_RECT;
}

void CurveDataFixedX::append( const QPointF &point ) {
    if (keepHiddenPoints_|| !QWT_SERIES_BOUNDING_RECT.isValid() || (point.x() >= left && point.x() <= right)) {
        QWT_SERIES_SAMPLES += point;

        if (QWT_SERIES_BOUNDING_RECT.isValid()) {
            if (point.y() > QWT_SERIES_BOUNDING_RECT.bottom()) {
                QWT_SERIES_BOUNDING_RECT.setBottom(point.y());
            } else if (point.y() < QWT_SERIES_BOUNDING_RECT.top()) {
                QWT_SERIES_BOUNDING_RECT.setTop(point.y());
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
    if (!QWT_SERIES_BOUNDING_RECT.isValid()) {
        QWT_SERIES_BOUNDING_RECT = CurveDataBase::boundingRect();
        QWT_SERIES_BOUNDING_RECT.setBottom(top);
        QWT_SERIES_BOUNDING_RECT.setTop(bottom);
    }
    return QWT_SERIES_BOUNDING_RECT;
}

void CurveDataFixedY::append( const QPointF &point ) {
    if (keepHiddenPoints_|| !QWT_SERIES_BOUNDING_RECT.isValid() || (point.y() >= bottom && point.y() <= top)) {
        QWT_SERIES_SAMPLES += point;

        if (QWT_SERIES_BOUNDING_RECT.isValid()) {
            if (point.x() > QWT_SERIES_BOUNDING_RECT.right()) {
                QWT_SERIES_BOUNDING_RECT.setRight(point.x());
            } else if (point.x() < QWT_SERIES_BOUNDING_RECT.left()) {
                QWT_SERIES_BOUNDING_RECT.setLeft(point.x());
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
    if (!QWT_SERIES_BOUNDING_RECT.isValid()) {
        QWT_SERIES_BOUNDING_RECT = CurveDataBase::boundingRect();
        if (QWT_SERIES_BOUNDING_RECT.isValid()) {
            QWT_SERIES_BOUNDING_RECT.setLeft(QWT_SERIES_BOUNDING_RECT.right() - timespan_);
        }
    }
    return QWT_SERIES_BOUNDING_RECT;
}

void CurveDataTime::append( const QPointF &point ) {
    if (keepHiddenPoints_|| !QWT_SERIES_BOUNDING_RECT.isValid() || point.x() >= QWT_SERIES_BOUNDING_RECT.left()) {
        QWT_SERIES_SAMPLES += point;

        if (QWT_SERIES_BOUNDING_RECT.isValid()) {
            if (point.x() > QWT_SERIES_BOUNDING_RECT.right()) {
                QWT_SERIES_BOUNDING_RECT.setRight(point.x());
                QWT_SERIES_BOUNDING_RECT.setLeft(QWT_SERIES_BOUNDING_RECT.right() - timespan_);

                if (!keepHiddenPoints_) {
                    // delete old points which are not inside bounding rectangle anymore
                    // We assume points to be chronologically ordered.
                    QVector<QPointF>::Iterator iter = QWT_SERIES_SAMPLES.begin();
                    while(iter != QWT_SERIES_SAMPLES.end()) {
                        if (QWT_SERIES_BOUNDING_RECT.contains(*iter)) break;
                        iter++;
                    }
                    QWT_SERIES_SAMPLES.erase(QWT_SERIES_SAMPLES.begin(), iter);
                }
            }

            if (point.y() > QWT_SERIES_BOUNDING_RECT.bottom()) {
                QWT_SERIES_BOUNDING_RECT.setBottom(point.y());
            } else if (point.y() < QWT_SERIES_BOUNDING_RECT.top()) {
                QWT_SERIES_BOUNDING_RECT.setTop(point.y());
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
    if (!QWT_SERIES_BOUNDING_RECT.isValid()) {
        QWT_SERIES_BOUNDING_RECT = CurveDataBase::boundingRect();
        if (QWT_SERIES_BOUNDING_RECT.isValid()) {
            QWT_SERIES_BOUNDING_RECT.setLeft(QWT_SERIES_BOUNDING_RECT.right() - timespan_);
            QWT_SERIES_BOUNDING_RECT.setBottom(top);
            QWT_SERIES_BOUNDING_RECT.setTop(bottom);
        }
    }
    return QWT_SERIES_BOUNDING_RECT;
}

void CurveDataTimeFixedY::append( const QPointF &point ) {
    if (keepHiddenPoints_|| !QWT_SERIES_BOUNDING_RECT.isValid() || (point.x() >= QWT_SERIES_BOUNDING_RECT.left() && point.y() >= bottom && point.y() <= top)) {
        QWT_SERIES_SAMPLES += point;

        if (QWT_SERIES_BOUNDING_RECT.isValid()) {
            if (point.x() > QWT_SERIES_BOUNDING_RECT.right()) {
                QWT_SERIES_BOUNDING_RECT.setRight(point.x());
                QWT_SERIES_BOUNDING_RECT.setLeft(QWT_SERIES_BOUNDING_RECT.right() - timespan_);

                if (!keepHiddenPoints_) {
                    // delete old points which are not inside bounding rectangle anymore
                    // We assume points to be chronologically ordered.
                    QVector<QPointF>::Iterator iter = QWT_SERIES_SAMPLES.begin();
                    while(iter != QWT_SERIES_SAMPLES.end()) {
                        if (QWT_SERIES_BOUNDING_RECT.contains(*iter)) break;
                        iter++;
                    }
                    QWT_SERIES_SAMPLES.erase(QWT_SERIES_SAMPLES.begin(), iter);
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
QRectF CurveDataFixedXFixedY::boundingRect() const {
    if (!QWT_SERIES_BOUNDING_RECT.isValid()) {
        QWT_SERIES_BOUNDING_RECT.setLeft(left);
        QWT_SERIES_BOUNDING_RECT.setRight(right);
        QWT_SERIES_BOUNDING_RECT.setBottom(top);
        QWT_SERIES_BOUNDING_RECT.setTop(bottom);
	}
    return QWT_SERIES_BOUNDING_RECT;
}
void CurveDataFixedXFixedY::append( const QPointF &point ) {
    if (keepHiddenPoints_|| (
                point.y() >= QWT_SERIES_BOUNDING_RECT.top() &&
                point.y() <= QWT_SERIES_BOUNDING_RECT.bottom() &&
                point.x() >= QWT_SERIES_BOUNDING_RECT.left() &&
                point.x() <= QWT_SERIES_BOUNDING_RECT.right())) {
        QWT_SERIES_SAMPLES += point;
    }
}

