#include "oscilloscope.h"
#include <QVBoxLayout>
#include <QTabWidget>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QWidget>
#include <QFrame>
#include <QSettings>


Oscilloscope::Oscilloscope(QWidget *parent) :
    BaseFrontend("Serielles Oszilloskop", parent), interface(nullptr)
{
    plot = new DataGraph(this);
    tab = new QTabWidget;

    QGridLayout* hlayout = new QGridLayout;
    hlayout->addWidget(plot, 0, 0);
    hlayout->addWidget(tab, 0, 1);
    hlayout->setColumnStretch(0, 1);
    setLayout(hlayout);

    QGridLayout* textlayout = new QGridLayout;
    QLabel* label = new QLabel("Kommazeichen");
    textlayout->addWidget(label, 0, 0, Qt::AlignTop);
    comma_textbox = new QLineEdit;
    textlayout->addWidget(comma_textbox, 0 , 1, Qt::AlignTop);
    connect(comma_textbox, SIGNAL(editingFinished()), this, SLOT(onTextSettingsChanged()));

    label = new QLabel("Kanaltrenner");
    textlayout->addWidget(label, 1, 0, Qt::AlignTop);
    subdelim_textbox = new QLineEdit;
    textlayout->addWidget(subdelim_textbox, 1, 1, Qt::AlignTop);
    connect(subdelim_textbox, SIGNAL(editingFinished()), this, SLOT(onTextSettingsChanged()));

    subdelim_newline = new QCheckBox("Newline");
    textlayout->addWidget(subdelim_newline, 2, 1);
    connect(subdelim_newline, SIGNAL(toggled(bool)), this, SLOT(onTextSettingsChanged()));

    label = new QLabel("Datensatz-Trenner");
    textlayout->addWidget(label, 3, 0, Qt::AlignTop);
    delim_textbox = new QLineEdit;
    textlayout->addWidget(delim_textbox, 3, 1, Qt::AlignTop);
    connect(delim_textbox, SIGNAL(editingFinished()), this, SLOT(onTextSettingsChanged()));

    delim_newline = new QCheckBox("Newline");
    textlayout->addWidget(delim_newline, 4, 1);
    connect(delim_newline, SIGNAL(toggled(bool)), this, SLOT(onTextSettingsDelimNewLineChanged(bool)));
    delim_emptyline = new QCheckBox("Leere Zeile");
    textlayout->addWidget(delim_emptyline, 5, 1);
    connect(delim_emptyline, SIGNAL(toggled(bool)), this, SLOT(onTextSettingsDelimEmptyLineChanged(bool)));

    timechannel_checkbox = new QCheckBox("1. Kanal enthält Zeit");
    textlayout->addWidget(timechannel_checkbox, 6, 0, 1, 2, Qt::AlignTop);
    connect(timechannel_checkbox, SIGNAL(toggled(bool)), this, SLOT(onTextSettingsChanged()));

    QFrame* line = new QFrame();
    line->setFrameShape(static_cast<QFrame::Shape>(QFrame::HLine));
    line->setFrameShadow(QFrame::Sunken);
    textlayout->addWidget(line, 7, 0, 1, 2, Qt::AlignTop);

    label = new QLabel("Gefundene Kanäle:");
    textlayout->addWidget(label, 8, 0, Qt::AlignTop);
    channel_label = new QLabel("0");
    textlayout->addWidget(channel_label, 8, 1, Qt::AlignTop);

    textlayout->setRowStretch(textlayout->rowCount() - 1, 1);
    QWidget* textwidget = new QWidget;
    textwidget->setLayout(textlayout);
    tab->addTab(textwidget, "Text-Stream");
    
    connect(tab, SIGNAL(currentChanged(int)), this, SLOT(onStreamTypeChanged(int)));

    // this function constructs a DataPointInterface-Object in interface
    readSettings();

    addActions(plot->getActions());
}

Oscilloscope::~Oscilloscope() {
    writeSettings();
}

void Oscilloscope::onConnected(bool readOnly, bool isBuffered, QIODevice* dev) {
    (void)readOnly;
    (void)dev;
    hasBufferedConnection = isBuffered;
}

void Oscilloscope::onStreamTypeChanged(int index) {
    if (interface != nullptr) {
        interface->disconnect(this);
        delete interface;
    }
    
    switch (index) {
    case 0:
        TextDataPointInterface* new_interface = new TextDataPointInterface;
        connect(new_interface, SIGNAL(dataPointsReady(QList<DataEntry>)), this, SLOT(onDataPointsReady(QList<DataEntry>)));
        connect(new_interface, SIGNAL(channelsDetected(int)), this, SLOT(onTextChannelsDetected(int)));
        interface = new_interface;
        onTextSettingsChanged();
        break;
        
    }
    plot->clear();
}

void Oscilloscope::writeData(QByteArray data) {
    // buffered devices do always output their contents completely, so clear contents beforehand
    if (hasBufferedConnection) clear();

    interface->writeData(data);
}

void Oscilloscope::onDataPointsReady(QList<DataEntry> dataPoints) {
    for (DataEntry entry : dataPoints) {
        plot->addData(entry.channel_, QPointF(entry.time_, entry.data_));
    }
}


void Oscilloscope::clear(void) {
    interface->clear();
    plot->clear();
    channel_label->setText("0");
}

bool Oscilloscope::saveOutput(QString file) {
    return plot->saveOutput(file);
}

void Oscilloscope::readSettings() {
    QSettings settings;
    settings.beginGroup(objectName());
    
    tab->setCurrentIndex(settings.value("streamType", 0).toInt());
    onStreamTypeChanged(settings.value("streamType", 0).toInt());

    comma_textbox->setText(settings.value("decimalPoint", ".").toString());
    delim_textbox->setText(settings.value("delim", " ").toString());
    delim_newline->setChecked(settings.value("delimNewline", false).toBool());
    delim_emptyline->setChecked(settings.value("delimEmptyLine", false).toBool());
    subdelim_textbox->setText(settings.value("channelDelim", ",").toString());
    subdelim_newline->setChecked(settings.value("channelDelimNewline", false).toBool());
    timechannel_checkbox->setChecked(settings.value("firstChannelIsTime", false).toBool());
    onTextSettingsChanged();
}

void Oscilloscope::writeSettings() {
    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("streamType", tab->currentIndex());
    settings.setValue("decimalPoint", comma_textbox->text());
    settings.setValue("delim", delim_textbox->text());
    settings.setValue("delimNewline", delim_newline->isChecked());
    settings.setValue("delimEmptyLine", delim_emptyline->isChecked());
    settings.setValue("channelDelim", subdelim_textbox->text());
    settings.setValue("channelDelimNewline", subdelim_newline->isChecked());
    settings.setValue("firstChannelIsTime", timechannel_checkbox->isChecked());
}


// ------------------------------------------------------------------------------
// text-related functions
// ------------------------------------------------------------------------------

void Oscilloscope::onTextChannelsDetected(int amount) {
    plot->clear();

    for (int i=0; i < amount; ++i) {
        plot->addChannel(QString("%1").arg(i));
    }

    channel_label->setText(QString("%1").arg(amount));
}

void Oscilloscope::onTextSettingsChanged() {
    TextDataPointInterface* textinterface = static_cast<TextDataPointInterface*>(interface);

    if (delim_newline->isChecked()) {
        delim_textbox->setDisabled(true);
        textinterface->setDelim("\n");
    } else if (delim_emptyline->isChecked()) {
        delim_textbox->setDisabled(true);
        textinterface->setDelim("\n\n");
    } else {
        delim_textbox->setEnabled(true);
        if (delim_textbox->text().size() > 0) {
            textinterface->setDelim(delim_textbox->text().toLatin1());
        }
    }

    if (subdelim_newline->isChecked()) {
        subdelim_textbox->setDisabled(true);
        textinterface->setChannelDelim("\n");
    } else {
        subdelim_textbox->setEnabled(true);
        if (subdelim_textbox->text().size() > 0) {
            textinterface->setChannelDelim(subdelim_textbox->text().toLatin1());
        }
    }

    if (comma_textbox->text().size() > 0) {
        textinterface->setDecimalPoint(comma_textbox->text().at(0).toLatin1());
    }
    textinterface->setFirstChannelIsTime(timechannel_checkbox->isChecked());

    clear();
    emit requestData();
}


void Oscilloscope::onTextSettingsDelimNewLineChanged(bool checked) {
    if (checked) {
        delim_emptyline->setChecked(false);
    }
    onTextSettingsChanged();
}

void Oscilloscope::onTextSettingsDelimEmptyLineChanged(bool checked) {
    if (checked) {
        delim_newline->setChecked(false);
    }
    onTextSettingsChanged();

}






