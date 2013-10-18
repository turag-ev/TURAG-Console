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
#include <qwt_system_clock.h>


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

    label = new QLabel("Trenner");
    textlayout->addWidget(label, 1, 0, Qt::AlignTop);
    delim_textbox = new QLineEdit;
    textlayout->addWidget(delim_textbox, 1, 1, Qt::AlignTop);
    connect(delim_textbox, SIGNAL(editingFinished()), this, SLOT(onTextSettingsChanged()));

    delim_newline = new QCheckBox("Newline");
    textlayout->addWidget(delim_newline, 2, 1);
    connect(delim_newline, SIGNAL(toggled(bool)), this, SLOT(onTextSettingsDelimNewLineChanged(bool)));
    delim_emptyline = new QCheckBox("Leere Zeile");
    textlayout->addWidget(delim_emptyline, 3, 1);
    connect(delim_emptyline, SIGNAL(toggled(bool)), this, SLOT(onTextSettingsDelimEmptyLineChanged(bool)));

    label = new QLabel("Kanaltrenner");
    textlayout->addWidget(label, 4, 0, Qt::AlignTop);
    subdelim_textbox = new QLineEdit;
    textlayout->addWidget(subdelim_textbox, 4, 1, Qt::AlignTop);
    connect(subdelim_textbox, SIGNAL(editingFinished()), this, SLOT(onTextSettingsChanged()));

    subdelim_newline = new QCheckBox("Newline");
    textlayout->addWidget(subdelim_newline, 5, 1);
    connect(subdelim_newline, SIGNAL(toggled(bool)), this, SLOT(onTextSettingsChanged()));

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
}

Oscilloscope::~Oscilloscope() {
    writeSettings();
}

void Oscilloscope::onConnected(bool readOnly, bool isSequential) {
    (void)readOnly;
    hasSequentialConnection = isSequential;
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
    // sequential devices do always output their contents completely, so clear contents beforehand
    if (!hasSequentialConnection) clear();

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

bool Oscilloscope::saveOutput(void) {
    return plot->saveOutput();
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
        textinterface->setDelimNewline();
    } else if (delim_emptyline->isChecked()) {
        delim_textbox->setDisabled(true);
        textinterface->setDelimEmptyLine();
    } else {
        delim_textbox->setEnabled(true);
        if (delim_textbox->text().size() > 0) {
            textinterface->setDelim(delim_textbox->text().at(0).toLatin1());
        }
    }

    if (subdelim_newline->isChecked()) {
        subdelim_textbox->setDisabled(true);
        textinterface->setChannelDelimNewline();
    } else {
        subdelim_textbox->setEnabled(true);
        if (subdelim_textbox->text().size() > 0) {
            textinterface->setChannelDelim(subdelim_textbox->text().at(0).toLatin1());
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






// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// class TextDataPointInterface
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

TextDataPointInterface::TextDataPointInterface() :
    delim_(' '), delimIsNewline_(false), delimIsEmptyLine_(false),
    channelDelim_(','), ChannelDelimIsNewline_(false), decimalPoint_('.'),
    firstChannelIsTime_(false), detectChannels(true)
{
}


void TextDataPointInterface::setDecimalPoint(char decimalPoint) {
    decimalPoint_ = decimalPoint;
    buffer.clear();
    detectChannels = true;
}

void TextDataPointInterface::setDelim(char delim) {
    delim_ = delim;
    delimIsEmptyLine_ = false;
    delimIsNewline_ = false;
    buffer.clear();
    detectChannels = true;
}

void TextDataPointInterface::setDelimNewline() {
    delimIsEmptyLine_ = false;
    delimIsNewline_ = true;
    buffer.clear();
    detectChannels = true;
}

void TextDataPointInterface::setDelimEmptyLine() {
    delimIsEmptyLine_ = true;
    delimIsNewline_ = false;
    buffer.clear();
    detectChannels = true;
}

void TextDataPointInterface::setChannelDelim(char delim) {
    channelDelim_ = delim;
    ChannelDelimIsNewline_ = false;
    buffer.clear();
    detectChannels = true;
}

void TextDataPointInterface::setChannelDelimNewline() {
    ChannelDelimIsNewline_ = true;
    buffer.clear();
    detectChannels = true;
}

void TextDataPointInterface::setFirstChannelIsTime(bool isTime) {
    firstChannelIsTime_ = isTime;
    buffer.clear();
    detectChannels = true;
}

void TextDataPointInterface::writeData(QByteArray data) {
    // append new data to already buffered data, removing all line feeds
    buffer.append(data.replace("\r", ""));

    // start time if that is a fresh package
    if (detectChannels && !firstChannelIsTime_) {
        clock.start();
    }

    // search for packet delim
    while ((delimIsEmptyLine_ && buffer.contains("\n\n")) || (delimIsNewline_ && buffer.contains('\n')) || buffer.contains(delim_)) {
        QByteArray packet;

        if (delimIsEmptyLine_) {
            int index = buffer.indexOf("\n\n");
            packet = buffer.left(index);
            buffer.remove(0, index + 2);
        } else if (delimIsNewline_) {
            int index = buffer.indexOf('\n');
            packet = buffer.left(index);
            buffer.remove(0, index + 1);
        } else {
            int index = buffer.indexOf(delim_);
            packet = buffer.left(index);
            buffer.remove(0, index + 1);
        }

        char delim;
        if (ChannelDelimIsNewline_) {
            delim = '\n';
        } else {
            delim = channelDelim_;
        }
        QList<QByteArray> parts = packet.split(delim);

        // data invalid if we only have the time channel
        if (firstChannelIsTime_ && parts.size() <= 1) {
            break;
        }

        QList<DataEntry> list;

        float time = 0;
        int start_i = 0;

        if (firstChannelIsTime_) {
            start_i = 1;

            QByteArray part = parts.at(0).trimmed().replace("\n", "");
            bool success = false;
            time = part.toFloat(&success);

            if (!success) {
                break;
            }
        } else {
            time = clock.elapsed();
        }

        for (int i = start_i; i < parts.size(); ++i) {
            QByteArray part = parts.at(i).trimmed().replace("\n", "");
            bool success = false;
            float value = part.toFloat(&success);

            if (success) {
                list.append(DataEntry(i - start_i, time, value));
            }
        }
        if (detectChannels) {
            emit channelsDetected(parts.size() - start_i);
            detectChannels = false;
        }
        emit dataPointsReady(list);
    }
}

void TextDataPointInterface::clear() {
    buffer.clear();
    detectChannels = true;
}
