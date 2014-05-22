#include <qapplication.h>
#include <qevent.h>
#include <qwhatsthis.h>
#include <qpainter.h>
#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_scale_map.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include "canvaspicker.h"
#include <QPen>
#include <QRect>
#include <QDebug>

CanvasPicker::CanvasPicker( QwtPlot *plot ):
    QObject( plot ),
    d_selectedCurve( NULL ),
    d_selectedPoint( -1 )
{
    overlay = new SymbolOverlay(19, plot->canvas());

    setEnabled(true);

    // We want the focus, but no focus rect. The
    // selected point will be highlighted instead.

    QwtPlotCanvas *canvas = qobject_cast<QwtPlotCanvas *>( plot->canvas() );
    canvas->setFocusPolicy( Qt::StrongFocus );
    canvas->setFocusIndicator( QwtPlotCanvas::ItemFocusIndicator );
    canvas->setFocus();

    //    const char *text =
    //        "All points can be moved using the left mouse button "
    //        "or with these keys:\n\n"
    //        "- Up:\t\tSelect next curve\n"
    //        "- Down:\t\tSelect previous curve\n"
    //        "- Left, ´-´:\tSelect next point\n"
    //        "- Right, ´+´:\tSelect previous point\n"
    //        "- 7, 8, 9, 4, 6, 1, 2, 3:\tMove selected point";
    //    canvas->setWhatsThis( text );
}

QwtPlot *CanvasPicker::plot() {
    return qobject_cast<QwtPlot *>( parent() );
}

const QwtPlot *CanvasPicker::plot() const {
    return qobject_cast<const QwtPlot *>( parent() );
}


bool CanvasPicker::eventFilter( QObject *object, QEvent *event ) {
    if ( plot() == NULL || object != plot()->canvas() )
        return false;

    switch( event->type() ) {
    case QEvent::MouseButtonPress:
    {
        const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
        if (mouseEvent->button() == Qt::LeftButton) {
            selectManually( mouseEvent->pos() );
        }
        return true;
    }
    case QEvent::KeyPress:
    {
        const QKeyEvent *keyEvent = static_cast<QKeyEvent *>( event );

        switch( keyEvent->key() )
        {
        case Qt::Key_Up:
        {
            emit nextPlotCurveSuggested();
            return true;
        }
        case Qt::Key_Down:
        {
            emit previuosPlotCurveSuggested();
            return true;
        }
        case Qt::Key_Right:
        case Qt::Key_Plus:
        {
            if ( d_selectedCurve ) {
                int amount = 0;
                if (keyEvent->modifiers() & Qt::ShiftModifier) {
                    amount = 0.01 * d_selectedCurve->data()->size();
                }
                if (keyEvent->modifiers() & Qt::ControlModifier) {
                    amount = 0.05 * d_selectedCurve->data()->size();
                }
                if (!amount) {
                    amount = 1;
                }

                shiftPointCursor( amount );
            } else {
                emit previuosPlotCurveSuggested();
            }
            return true;
        }
        case Qt::Key_Left:
        case Qt::Key_Minus:
        {
            if ( d_selectedCurve ) {
                int amount = 0;
                if (keyEvent->modifiers() & Qt::ShiftModifier) {
                    amount = -0.01 * d_selectedCurve->data()->size();
                }
                if (keyEvent->modifiers() & Qt::ControlModifier) {
                    amount = -0.1 * d_selectedCurve->data()->size();
                }
                if (!amount) {
                    amount = -1;
                }
                shiftPointCursor( amount );
            } else {
                emit nextPlotCurveSuggested();
            }
            return true;
        }

        default:
            break;
        }
    }
    default:
        break;
    }

    return QObject::eventFilter( object, event );
}

// Select the point at a position. If there is no point
// deselect the selected point

void CanvasPicker::selectPlotCurve(QwtPlotCurve* curve) {
    d_selectedCurve = curve;
    showCursor(true);
}

void CanvasPicker::selectPlotPoint(int index) {
    d_selectedPoint = index;
    showCursor(true);
}

void CanvasPicker::selectManually( const QPoint &pos ) {
    if (!d_selectedCurve) {
        return;
    }

    int index = d_selectedCurve->closestPoint( pos);

    if (index != -1) {
        emit dataPointSuggested(index);
    }
}

// Hightlight the selected point
void CanvasPicker::showCursor( bool showIt ) {
    if ( !d_selectedCurve || d_selectedPoint == -1 )
        return;

    if ( showIt ) {
        QwtArraySeriesData<QPointF>* data = static_cast<QwtArraySeriesData<QPointF>*>(d_selectedCurve->data());
        QPointF point(data->sample(d_selectedPoint));
        //qDebug() << point;

        point.setX(plot()->transform(d_selectedCurve->xAxis(), point.x()));
        point.setY(plot()->transform(d_selectedCurve->yAxis(), point.y()));

        //qDebug() << point;

        overlay->showSymbol(point);
    } else {
        overlay->hide();
    }
}

// Select the next/previous neighbour of the selected point
void CanvasPicker::shiftPointCursor(int amount ) {
    if ( !d_selectedCurve )
        return;

    int index = d_selectedPoint + amount;
    index = ( index + d_selectedCurve->dataSize() ) % d_selectedCurve->dataSize();

    if ( index != d_selectedPoint ) {
        emit dataPointSuggested(index);
    }
}

void CanvasPicker::setEnabled(bool enabled) {
    QwtPlotCanvas *canvas = qobject_cast<QwtPlotCanvas *>( plot()->canvas() );

    if (enabled) {
        canvas->installEventFilter( this );
        showCursor( true );
    } else {
        canvas->removeEventFilter(this);
        showCursor( false );
    }
}


SymbolOverlay::SymbolOverlay(int size, QWidget *widget) :
    QwtWidgetOverlay(widget)
{
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(1);
    symbol = new QwtSymbol(QwtSymbol::XCross, Qt::NoBrush, pen, QSize(size+6, size+6));

    shadow = new QwtSymbol(QwtSymbol::Ellipse, QColor(255, 255, 0, 63), QColor(0, 0, 0, 63), QSize(size, size));
}

void SymbolOverlay::showSymbol(const QPointF & pos_) {
    pos = pos_;
    updateOverlay();
}

void SymbolOverlay::hideSymbol(void) {

}

QRegion SymbolOverlay::maskHint() const {
    QRect rect(shadow->boundingRect());
    rect.adjust(pos.x(), pos.y(), pos.x(), pos.y());

    //    return qwtMaskRegion(rect , symbol->pen().width());
    return QRegion(rect);
}

void SymbolOverlay::drawOverlay(QPainter *painter) const {
    shadow->drawSymbol(painter, pos);
    symbol->drawSymbol(painter, pos);
}
