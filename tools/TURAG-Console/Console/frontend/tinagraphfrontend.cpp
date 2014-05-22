#include "tinagraphfrontend.h"
#include "graphutils/datagraph.h"
#include <tina++/utils/base64.h>
#include <tina/debug/graph.h>
#include <QHBoxLayout>
#include <QListWidget>
#include <QTextStream>

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
}

void TinaGraphFrontend::writeLine(QByteArray line) {
    if (line.size() > 2) {
        char level = line.at(1);
        unsigned type = line.at(0);
        line.remove(0, 2);

        if (level == TURAG_DEBUG_GRAPH_PREFIX[0]) {
            bool ok = false;
            int space_pos = line.indexOf(' ');
            int index = line.left(space_pos).toInt(&ok);

            if (ok) {
                switch (type) {
                case TURAG_DEBUG_GRAPH_CREATE[0]: {
                    QString title = line.right(line.size() - space_pos - 1);
                    int oldIndex = graphIndices.indexOf(index);
                    if (oldIndex != -1) {
                        static_cast<DataGraph*>(stack->widget(oldIndex))->clear();
                        static_cast<DataGraph*>(stack->widget(oldIndex))->setTitle(title);
                        graphlist->item(oldIndex)->setText(QString("%1 - ").arg(index) + title);
                    } else {
                        stack->addWidget(new DataGraph(title));
                        graphIndices.append(index);
                        graphlist->addItem(QString("%1 - ").arg(index) + title);
                    }
                    emit newGraph(index);

                    break;
                }
                case TURAG_DEBUG_GRAPH_CHANNEL[0]: {
                    int listindex = graphIndices.indexOf(index);
                    if (listindex != -1) {
                        int secondSpacePos = line.indexOf(' ', space_pos + 1);
                        int timesize = line.mid(space_pos + 1, secondSpacePos - space_pos - 1).toInt(&ok);

                        if (ok) {
                            QString title = line.right(line.size() - secondSpacePos - 1);
                            if (timesize == 0) {
                                static_cast<DataGraph*>(stack->widget(listindex))->addChannel(title);
                            } else {
                                static_cast<DataGraph*>(stack->widget(listindex))->addChannel(title, static_cast<qreal>(timesize), false);
                            }
                        }

                    }
                    break;
                }
                case TURAG_DEBUG_GRAPH_CHANNEL_FIXED[0]: {
                    int listindex = graphIndices.indexOf(index);
                    if (listindex != -1) {
                        QTextStream stream(line);

                        int x_left, y_bottom;
                        unsigned width, height;

                        stream >> x_left;  // consume the index, which e don't need here
                        stream >> x_left;
                        stream >> y_bottom;
                        stream >> width;
                        stream >> height;

                        qDebug() << x_left << y_bottom << width << height;

                        QString title(stream.readLine());

                        static_cast<DataGraph*>(stack->widget(listindex))->addChannel(title, static_cast<qreal>(x_left), static_cast<qreal>(width), static_cast<qreal>(y_bottom), static_cast<qreal>(height));
                    }
                    break;
                }
                case TURAG_DEBUG_GRAPH_DATA[0]: {
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
                                //graph->doAutoZoom();

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
    DataGraph* graph = static_cast<DataGraph*>(stack->currentWidget());
    clearActions();
    if (graph) addActions(graph->getActions());
}

void TinaGraphFrontend::onConnected(bool , bool, QIODevice*) {
    
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

// needed for the interface
void TinaGraphFrontend::writeData(QByteArray data) {
    (void)data;
}


