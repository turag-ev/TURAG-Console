#include "tinagraphfrontend.h"
#include "util/datagraph.h"
#include <tina++/utils/base64.h>
#include <QHBoxLayout>
#include <QAction>
#include <QActionGroup>
#include <QSettings>
#include <QListWidget>

TinaGraphFrontend::TinaGraphFrontend(QWidget *parent) :
    BaseFrontend("TinaGraphFrontend", parent)
{
    QHBoxLayout* layout = new QHBoxLayout;

    graphlist = new QListWidget;
    layout->addWidget(graphlist);
    connect(graphlist, SIGNAL(currentRowChanged(int)), this, SLOT(activateGraphInternal(int)));

    stack = new QStackedWidget;
    layout->addWidget(stack);
    layout->setStretch(1, 2);

    setLayout(layout);

    QAction* separator_action = new QAction(this);
    separator_action->setSeparator(true);
    addAction(separator_action);

    QAction* save_action = new QAction("Save Graph", this);
    addAction(save_action);
    connect(save_action, SIGNAL(triggered()), this, SLOT(saveOutput()));

    QAction* zoom_fit_action = new QAction("Apply automatic zoom", this);
    addAction(zoom_fit_action);
    connect(zoom_fit_action, SIGNAL(triggered()), this, SLOT(onZoomAuto()));

    QActionGroup* zoom_group = new QActionGroup(this);

    zoom_box_zoom_action = new QAction("Activate box zoom", this);
    zoom_box_zoom_action->setActionGroup(zoom_group);
    connect(zoom_box_zoom_action, SIGNAL(triggered()), this, SLOT(onSetZoomer()));
    zoom_box_zoom_action->setCheckable(true);

    zoom_drag_action = new QAction("Activate graph panner", this);
    connect(zoom_drag_action, SIGNAL(triggered()), this, SLOT(onDragGraph()));
    zoom_drag_action->setActionGroup(zoom_group);
    zoom_drag_action->setCheckable(true);

    addActions(zoom_group->actions());

    QSettings settings;
    settings.beginGroup(objectName());
    zoom_drag_action->setChecked(settings.value("zoom_drag_action", true).toBool());
    zoom_box_zoom_action->setChecked(!(settings.value("zoom_drag_action", true).toBool()));

    if (zoom_drag_action->isChecked()) {
        onDragGraph();
    } else {
        onSetZoomer();
    }
}

TinaGraphFrontend::~TinaGraphFrontend(void) {
    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("zoom_drag_action", zoom_drag_action->isChecked());
}

void TinaGraphFrontend::writeLine(QByteArray line) {
    if (line.size() > 2) {
        char level = line.at(1);
        unsigned type = line.at(0);
        line.remove(0, 2);

        if (level == 'D') {
            bool ok = false;
            int space_pos = line.indexOf(' ');
            int index = line.left(space_pos).toInt(&ok);

            if (ok) {
                switch (type) {
                case 'n':
                    if (graphIndices.indexOf(index) == -1) {
                        QString title = line.right(line.size() - space_pos - 1);
                        stack->addWidget(new DataGraph(title));
                        graphIndices.append(index);
                        graphlist->addItem(QString("%1 - ").arg(index) + title);
                        emit newGraph(index);
                    }
                    break;

                case 'b': {
                    int listindex = graphIndices.indexOf(index);
                    if (listindex != -1) {
                        QString title = line.right(line.size() - space_pos - 1);
                        static_cast<DataGraph*>(stack->widget(listindex))->addChannel(title);
                    }
                    break;
                }
                case 'd': {
                    int listindex = graphIndices.indexOf(index);
                    if (listindex != -1) {
                        QString encoded = line.right(line.size() - space_pos - 1);
                        QByteArray byte_data = encoded.toLatin1();
                        const uint8_t* data = reinterpret_cast<const uint8_t*>(byte_data.constData());
                        size_t encoded_values = encoded.size() / 6;

                        if (encoded_values >= 2) {
                            float time;
                            TURAG::Base64::decode(data, 6, reinterpret_cast<uint8_t*>(&time));
                            data += 6;

                            DataGraph* graph = static_cast<DataGraph*>(stack->widget(listindex));
                            int channel = 0;

                            for (unsigned i = 1; i < encoded_values; ++i) {
                                float value;
                                TURAG::Base64::decode(data, 6, reinterpret_cast<uint8_t*>(&value));
                                graph->addData(channel, QPointF(time, value));
                                graph->doAutoZoom();

                                ++channel;
                                data += 6;
                            }
                        }
                    }
                    break;
                }
                }
            }
        }
    }
}


void TinaGraphFrontend::activateGraph(int index) {
    int listindex = graphIndices.indexOf(index);
    if (listindex != -1) {
        activateGraphInternal(listindex);
    }
}

void TinaGraphFrontend::activateGraphInternal(int index) {
    stack->setCurrentIndex(index);
    graphlist->setCurrentRow(index);
    if (zoom_box_zoom_action->isChecked()) {
        onSetZoomer();
    } else {
        onDragGraph();
    }
}

void TinaGraphFrontend::onConnected(bool , bool , QIODevice*) {
    
}


void TinaGraphFrontend::onDisconnected(bool ) {
    
}

void TinaGraphFrontend::clear(void) {
    QWidget* widget;
    while ((widget = stack->currentWidget())) {
        stack->removeWidget(widget);
        widget->deleteLater();
    }
    graphIndices.clear();
    graphlist->clear();
}

bool TinaGraphFrontend::saveOutput(void) {
    if (stack->count()) {
        DataGraph* graph = static_cast<DataGraph*>(stack->currentWidget());
        return graph->saveOutput();
    } else {
        return false;
    }
}

// needed for the interface
void TinaGraphFrontend::writeData(QByteArray data) {
    (void)data;
}


void TinaGraphFrontend::onZoomAuto(void) {
    if (stack->count()) {
        DataGraph* graph = static_cast<DataGraph*>(stack->currentWidget());
        graph->doAutoZoom();
    }
}


void TinaGraphFrontend::onSetZoomer(void) {
    if (stack->count()) {
        DataGraph* graph = static_cast<DataGraph*>(stack->currentWidget());
        graph->setZoomer();
    }
}

void TinaGraphFrontend::onDragGraph(void) {
    if (stack->count()) {
        DataGraph* graph = static_cast<DataGraph*>(stack->currentWidget());
        graph->setPanner();
    }
}
