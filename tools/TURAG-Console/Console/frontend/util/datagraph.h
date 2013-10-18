#ifndef DATAGRAPH_H
#define DATAGRAPH_H

#include <qwt_plot.h>
#include <QWidget>
#include <QList>

class QString;
class QPointF;
class QwtPlotCurve;
class QwtPlotZoomer;
class QwtPlotItem;



class DataGraph : public QwtPlot
{
    Q_OBJECT

protected:
    QList<QwtPlotCurve*> channels;
    QwtPlotZoomer* zoomer;

    virtual void updateCurveColors();

public:
    explicit DataGraph(QWidget *parent = 0);

    int getNumberOfChannels() const;
    QString getChannelTitle(int index) const;

protected slots:
    void showCurve(QwtPlotItem *item, bool on);
#if QWT_VERSION >= 0x060100
    void legendChecked(const QVariant &itemInfo, bool on);
#endif

public slots:
    virtual void clear();
    virtual void addChannel(QString title);
    virtual void removeChannel(int index);
    virtual void addData(int channel, QPointF data);
    virtual bool saveOutput();
};




#endif // DATAGRAPH_H
