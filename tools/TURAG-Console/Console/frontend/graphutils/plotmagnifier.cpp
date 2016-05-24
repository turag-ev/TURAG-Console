#include "plotmagnifier.h"

#include <QWheelEvent>
#include <qwt_plot.h>
#include <qwt_scale_div.h>
#include <qwt_scale_map.h>

#include <QDebug>


PlotMagnifier::PlotMagnifier(QWidget * canvas_) :
	QwtPlotMagnifier (canvas_)
{
}


void PlotMagnifier::rescale(double factor)
{
	QwtPlot* plt = plot();
	if ( plt == NULL )
		return;

	factor = qAbs( factor );
	if ( factor == 1.0 || factor == 0.0 )
		return;

	bool doReplot = false;

	const bool autoReplot = plt->autoReplot();
	plt->setAutoReplot( false );

	for ( int axisId = 0; axisId < QwtPlot::axisCnt; axisId++ )
	{
		const QwtScaleDiv &scaleDiv = plt->axisScaleDiv( axisId );
		if ( isAxisEnabled( axisId ) )
		{
			QwtScaleMap map;
			map.setScaleInterval(scaleDiv.interval().minValue(), scaleDiv.interval().maxValue());
			double center;

			if (axisId == QwtPlot::xTop || axisId == QwtPlot::xBottom) {
				map.setPaintInterval(0, canvas()->width());
				center = map.invTransform(mousePos.x());
			} else {
				map.setPaintInterval(canvas()->height(), 0);
				center = map.invTransform(mousePos.y());
			}

			double minZoomed = center - ((center - scaleDiv.interval().minValue()) * factor);
			double maxZoomed = center + ((scaleDiv.interval().maxValue() - center) * factor);

			plt->setAxisScale( axisId, minZoomed, maxZoomed );
			doReplot = true;
		}
	}

	plt->setAutoReplot( autoReplot );

	if ( doReplot )
		plt->replot();
}

bool PlotMagnifier::eventFilter(QObject * object, QEvent * event_)
{
	if (object == canvas() && event_->type() == QEvent::Wheel) {
		QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event_);
		mousePos = wheelEvent->pos();
	}
	return QwtPlotMagnifier::eventFilter(object, event_);
}




