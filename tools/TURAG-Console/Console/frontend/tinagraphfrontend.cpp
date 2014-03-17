#include "tinagraphfrontend.h"
#include "util/datagraph.h"
#include <tina++/utils/base64.h>
#include <QHBoxLayout>
#include <QAction>

TinaGraphFrontend::TinaGraphFrontend(QWidget *parent) :
    BaseFrontend("TinaGraphFrontend", parent)
{
    stack = new QStackedWidget;
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(stack);
    setLayout(layout);

    QAction* separator_action = new QAction(this);
    separator_action->setSeparator(true);
    addAction(separator_action);

    QAction* save_action = new QAction("Save Graph", this);
    addAction(save_action);
    connect(save_action, SIGNAL(triggered()), this, SLOT(saveOutput()));
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
                        stack->addWidget(new DataGraph(line.right(line.size() - space_pos - 1)));
                        graphIndices.append(index);
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

                            qDebug() << "size=" << encoded.size() << QString(reinterpret_cast<const char*>(data));

                            for (unsigned i = 1; i < encoded_values; ++i) {
                                float value;
                                int result = TURAG::Base64::decode(data, 6, reinterpret_cast<uint8_t*>(&value));
                                graph->addData(channel, QPointF(time, value));

                                qDebug() << i << time << value << result;

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
        stack->setCurrentIndex(listindex);
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
}

bool TinaGraphFrontend::saveOutput(void) {
    DataGraph* graph = static_cast<DataGraph*>(stack->currentWidget());
    return graph->saveOutput();
}

// needed for the interface
void TinaGraphFrontend::writeData(QByteArray data) {
    (void)data;
}
