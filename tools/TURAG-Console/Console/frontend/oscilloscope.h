#ifndef TURAG_CONSOLE_CONSOLE_FRONTEND_OSCILLOSCOPE_H
#define TURAG_CONSOLE_CONSOLE_FRONTEND_OSCILLOSCOPE_H

#include "basefrontend.h"
#include "graphutils/datagraph.h"
#include <QList>
#include "util/datapointinterface.h"

class LineEditExt;
class CheckBoxExt;
class QLabel;
class QPointF;
class DataPointInterface;
class QTabWidget;
class QByteArray;


class Oscilloscope : public BaseFrontend {
    Q_OBJECT

public:
	explicit Oscilloscope(QWidget *parent = 0);
	~Oscilloscope();

public slots:
	virtual void writeData(QByteArray data);
	virtual void clear(void);
	virtual void onConnected(bool readOnly, QIODevice*);

protected slots:
	void onStreamTypeChanged(int index);
	void onDataPointsReady(QList<DataEntry> dataPoints);

	void onTextChannelsDetected(int amount);
	void onTextSettingsChanged();
	void onTextSettingsDelimNewLineChanged(bool checked);
	void onTextSettingsDelimEmptyLineChanged(bool checked);

protected:
    DataGraph* plot;
    QTabWidget* tab;
    LineEditExt* comma_textbox;
    LineEditExt* delim_textbox;
    CheckBoxExt* delim_newline;
    CheckBoxExt* delim_emptyline;
    LineEditExt* subdelim_textbox;
    CheckBoxExt* subdelim_newline;
    CheckBoxExt* timechannel_checkbox;
    QLabel* channel_label;
    
    DataPointInterface* interface;

    void readSettings();

};



#endif // TURAG_CONSOLE_CONSOLE_FRONTEND_OSCILLOSCOPE_H
