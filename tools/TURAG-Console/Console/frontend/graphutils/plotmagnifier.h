#ifndef PLOTMAGNIFIER_H
#define PLOTMAGNIFIER_H

#include <qwt_plot_magnifier.h>
#include <QPoint>

class QEvent;


class PlotMagnifier : public QwtPlotMagnifier
{
public:
	explicit PlotMagnifier( QWidget * );
	virtual ~PlotMagnifier() { }

protected:
	virtual void rescale( double factor );
	virtual bool eventFilter( QObject *, QEvent * );

private:
	QPoint mousePos;
};

#endif // PLOTMAGNIFIER_H
