#include "tinagraphfrontend.h"
#include "graphutils/datagraph.h"
#include <tina++/utils/base64.h>
#include <tina++/debug/graph.h>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QTextStream>
#include <QTreeWidgetItem>

TinaGraphFrontend::TinaGraphFrontend(QWidget *parent) :
    BaseFrontend("TinaGraphFrontend", parent)
{
    QHBoxLayout* layout = new QHBoxLayout;

    graphList = new QTreeWidget;
    graphList->setColumnCount(1);
    layout->addWidget(graphList);
    connect(graphList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(activateItem(QTreeWidgetItem*)));

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

                        delete graphList->takeTopLevelItem(oldIndex);
                        QTreeWidgetItem* item = createGraphEntry(index, title);
                        graphList->insertTopLevelItem(oldIndex, item);
                    } else {
                        stack->addWidget(new DataGraph(title));
                        graphIndices.append(index);

                        QTreeWidgetItem* item = createGraphEntry(index, title);
                        graphList->addTopLevelItem(item);
                    }
                    emit newGraph(index);

                    break;
                }

				case TURAG_DEBUG_GRAPH_COPY[0]: {
					int secondSpacePos = line.indexOf(' ', space_pos + 1);
					int oldChannelIndex = line.mid(space_pos + 1, secondSpacePos - space_pos - 1).toInt(&ok);
					int oldGraphIndex = graphIndices.indexOf(oldChannelIndex);
					QString newTitle = line.right(line.size() - secondSpacePos - 1);

					// only continue if old index exists and new does not
					if (oldGraphIndex != -1 && graphIndices.indexOf(index) == -1) {
						DataGraph* oldGraph = static_cast<DataGraph*>(stack->widget(oldGraphIndex));
						DataGraph* newGraph = new DataGraph;
						newGraph->copyMetadata(*oldGraph);
						newGraph->setTitle(newTitle);

						stack->addWidget(newGraph);
						graphIndices.append(index);

						QTreeWidgetItem* item = createGraphEntry(index, newTitle);
						item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
						graphList->addTopLevelItem(item);

						for (int i = 0; i < graphList->topLevelItem(oldGraphIndex)->child(0)->childCount(); ++i) {
							QTreeWidgetItem* groupItem = new QTreeWidgetItem(item->child(0));
							groupItem->setText(0, graphList->topLevelItem(oldGraphIndex)->child(0)->child(0)->text(0));
						}
					}

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

                        stream >> x_left;  // consume the index, which we don't need here
                        stream >> x_left;
                        stream >> y_bottom;
                        stream >> width;
                        stream >> height;

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

                                ++channel;
                                data += 6;
                            }
                        }
                    }
                    break;
                }

				case TURAG_DEBUG_GRAPH_DATA2D[0]: {
					int listindex = graphIndices.indexOf(index);
					if (listindex != -1) {
						DataGraph* graph = static_cast<DataGraph*>(stack->widget(listindex));
						int secondSpacePos = line.indexOf(' ', space_pos + 1);
						int channelIndex = line.mid(space_pos + 1, secondSpacePos - space_pos - 1).toInt(&ok);

						if (ok && channelIndex < graph->getNumberOfChannels()) {
							QString encoded = line.right(line.size() - secondSpacePos - 1);
							QByteArray byte_data = encoded.toLatin1();
							const uint8_t* data = reinterpret_cast<const uint8_t*>(byte_data.constData());
							size_t encoded_values = encoded.size() / 6;

							if (encoded_values == 2) {
								float x, y;

								TURAG::Base64::decode(data, 6, reinterpret_cast<uint8_t*>(&x));
								data += 6;
								TURAG::Base64::decode(data, 6, reinterpret_cast<uint8_t*>(&y));

								graph->addData(channelIndex, QPointF(x, y));
							}
						}
					}
					break;
				}

                case TURAG_DEBUG_GRAPH_VERTICAL_MARKER[0]: {
                    int listindex = graphIndices.indexOf(index);
                    if (listindex != -1) {
                        QTextStream stream(line);

                        QByteArray encoded;
                        int index;

                        stream >> index;  // consume the index, which we don't need here
                        stream >> encoded;

                        float time;
                        if (encoded.size() != TURAG::Base64::encodeLength(sizeof(time))) {
                            qDebug() << "Error: couldn't decode time";
                            return;
                        }

                        const uint8_t* data = reinterpret_cast<const uint8_t*>(encoded.constData());
                        TURAG::Base64::decode(data, 6, reinterpret_cast<uint8_t*>(&time));

                        QString title(stream.readLine());

                        static_cast<DataGraph*>(stack->widget(listindex))->addVerticalMarker(time);

                        QTreeWidgetItem* item = graphList->topLevelItem(listindex);
                        item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
                        QTreeWidgetItem* markerItem = new QTreeWidgetItem(item->child(1));
                        markerItem->setText(0, title);
                    }
                    break;
                }

                case TURAG_DEBUG_GRAPH_CHANNELGROUP[0]: {
                    int listindex = graphIndices.indexOf(index);
                    if (listindex != -1) {
                        QTextStream stream(line);

                        int index, count;

                        stream >> index;  // consume the index, which we don't need here
                        stream >> count;

                        QList<int> indices;

                        for (int i = 0; i < count; ++i) {
                            stream >> index;
                            indices.append(index);
                        }
                        QString title(stream.readLine());

                        static_cast<DataGraph*>(stack->widget(listindex))->addChannelGroup(indices);

                        QTreeWidgetItem* item = graphList->topLevelItem(listindex);
                        item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
                        QTreeWidgetItem* groupItem = new QTreeWidgetItem(item->child(0));
                        groupItem->setText(0, title);

                    }
                    break;
                }

                }
            }
        }
    }
}

QTreeWidgetItem* TinaGraphFrontend::createGraphEntry(int index, const QString& title) {
    QTreeWidgetItem* item = new QTreeWidgetItem;
    item->setText(0, QString("%1 - ").arg(index) + title);
    item->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);

    QTreeWidgetItem* viewItem = new QTreeWidgetItem(item);
    viewItem->setText(0, "Ansichten");

    QTreeWidgetItem* viewAllItem = new QTreeWidgetItem(viewItem);
    viewAllItem->setText(0, "Alle Kanäle anzeigen");

//    QTreeWidgetItem* markerItem = new QTreeWidgetItem(item);
//    markerItem->setText(0, "Marker");

    return item;
}

// maps tina graph indices on the QList-indices
void TinaGraphFrontend::activateGraph(int tina_index) {
    int listindex = graphIndices.indexOf(tina_index);
    if (listindex != -1 && listindex < graphList->topLevelItemCount()) {
        QTreeWidgetItem* item = graphList->topLevelItem(listindex);
        graphList->setCurrentItem(item);
        activateItem(item);
    }
}


void TinaGraphFrontend::activateItem(QTreeWidgetItem* item) {
    if (!item) return;

    int graphIndex = -1;
    if (!item->parent()) {
        graphIndex = item->treeWidget()->indexOfTopLevelItem(item);
    } else {
        if (item->parent()->parent()) {
            graphIndex = item->treeWidget()->indexOfTopLevelItem(item->parent()->parent());
        }
    }
    if (graphIndex == -1) {
        return;
    }

    stack->setCurrentIndex(graphIndex);
    DataGraph* graph = static_cast<DataGraph*>(stack->currentWidget());

    if (item->parent() && item->parent()->parent()) {
        if (item->parent() == item->parent()->parent()->child(0)) {
            if (item->parent()->indexOfChild(item) == 0) {
                graph->resetChannelGrouping();
            } else {
                graph->applyChannelGrouping(item->parent()->indexOfChild(item) - 1);
            }
        } else if (item->parent() == item->parent()->parent()->child(0)) {

        }
    }

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
    graphList->clear();
}

// needed for the interface
void TinaGraphFrontend::writeData(QByteArray data) {
    (void)data;
}


