#include "tinacamerafrontend.h"
#include "util/datagraph.h"
#include <tina++/utils/base64.h>
#include <QHBoxLayout>
#include <QListWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPushButton>

TinaCameraFrontend::TinaCameraFrontend(QWidget *parent) :
    BaseFrontend("TinaCameraFrontend", parent)
{
    QVBoxLayout* layout = new QVBoxLayout;

    scene = new QGraphicsScene(this);
    scene->addText("Waiting for connection ...");

    view = new QGraphicsView(scene);

    QHBoxLayout *buttonBoxA = new QHBoxLayout;

    QPushButton* b_dump = new QPushButton(style()->standardIcon(QStyle::SP_BrowserReload), "Dump &one");
    connect(b_dump, SIGNAL(pressed()), this, SLOT(handleButtonDumpOne()));
    buttonBoxA->addWidget(b_dump);

    QPushButton* b_dumpa = new QPushButton(style()->standardIcon(QStyle::SP_MediaPlay), "Dump &images");
    b_dumpa->setCheckable(true);
    connect(b_dumpa, SIGNAL(toggled(bool)), this, SLOT(handleButtonDumpAll(bool)));
    buttonBoxA->addWidget(b_dumpa);

    QHBoxLayout *buttonBoxB = new QHBoxLayout;

    QPushButton* b_roi = new QPushButton(style()->standardIcon(QStyle::SP_TrashIcon), "&Show ROIs");
    b_roi->setCheckable(true);
    connect(b_roi, SIGNAL(toggled(bool)), this, SLOT(handleButtonROIs(bool)));
    buttonBoxB->addWidget(b_roi);

    QPushButton* b_fs = new QPushButton(style()->standardIcon(QStyle::SP_MessageBoxInformation), "&Farbsensor info");
    connect(b_fs, SIGNAL(pressed()), this, SLOT(handleButtonFarbsensorInfo()));
    buttonBoxB->addWidget(b_fs);

    layout->addWidget(view);
    layout->addLayout(buttonBoxA);
    layout->addLayout(buttonBoxB);

    setLayout(layout);
}

void TinaCameraFrontend::writeLine(QByteArray line) {
    if (line.size() > 2) {
        char level = line.at(1);
        unsigned type = line.at(0);
        line.remove(0, 2);

        if (level == 'B') {
            switch (type) {
            case ':': { // image transmission
                // split at space
                QStringList bananasplit = QString(line).split(' ');
                if (bananasplit.count() != 7) {
                    qDebug() << "TinaCameraFrontend: invalid image_transmit packet! elements:" << bananasplit.count();
                    break;
                }

                // parse tokens
                bool tmpok = false, allok = true;

                QString channel_name = bananasplit.at(0);
                unsigned timestamp_ticks = bananasplit.at(1).toInt(&tmpok);
                allok = allok && tmpok;
                QString colorspace_abbrev = bananasplit.at(2);
                unsigned resolution_x = bananasplit.at(3).toInt(&tmpok);
                allok = allok && tmpok;
                unsigned resolution_y = bananasplit.at(4).toInt(&tmpok);
                allok = allok && tmpok;
                unsigned resolution_depth = bananasplit.at(5).toInt(&tmpok);
                allok = allok && tmpok;
                QString image_buffer_b64 = bananasplit.at(6);

                if (!allok) {
                    qDebug() << "TinaCameraFrontend: error parsing image_transmit packet!";
                    break;
                }

                qDebug() << "TinaCameraFrontend Image Dump: Channel:" << channel_name << "Time:" << timestamp_ticks <<
                            "Colorspace:" << colorspace_abbrev << " Resolution:" << resolution_x << "x" << resolution_y << "px," << resolution_depth << "bpp";

                // sanity check on parameters
                if (colorspace_abbrev != "R") {
                    qDebug() << "TinaCameraFrontend: only RGB colorspace supported!";
                    break;
                }
                if (resolution_x != 160 || resolution_y != 120 || resolution_depth != 2) {
                    qDebug() << "TinaCameraFrontend: invalid image resolution for STMCam!";
                    break;
                }

                // decode image data
                size_t pixels = image_buffer_b64.size() * 2 / 3;
                size_t desired_pixels = resolution_x*resolution_y*resolution_depth;
                if (pixels != desired_pixels) {
                    qDebug() << "TinaCameraFrontend: image data doesn't match given resolution, got" << pixels << "bytes but wanted" << desired_pixels;
                    break;
                }

                // cast source data array around for b64 decoding
                QByteArray source_data = image_buffer_b64.toUtf8();
                const uint8_t* source_data_cu = reinterpret_cast<const uint8_t*>(source_data.constData());

                // target buffer for decoded data
                QByteArray decoded_data;

                // decode blockwise
                constexpr unsigned block_size = 6; // must match size on other side
                for (unsigned i = 0; i < image_buffer_b64.size()/block_size; i++) {
                    // buffer for decoded block
                    uint8_t tmpbuf[block_size*2/3+1] = {0};

                    TURAG::Base64::decode(&source_data_cu[i*block_size], block_size, tmpbuf);
                    decoded_data.append(reinterpret_cast<const char*>(&tmpbuf), block_size*2/3);
                }

                if (decoded_data.size() != static_cast<int>(desired_pixels)) {
                    qDebug() << "TinaCameraFrontend: decoding fuckup, got" << decoded_data.size() << "bytes, but wanted" << desired_pixels;
                    break;
                }

                QImage image(reinterpret_cast<const uchar*>(decoded_data.constData()), resolution_x, resolution_y, QImage::Format_RGB16);
                updateImage(image);

                break;
            }
            }
        }
    }
}

void TinaCameraFrontend::onConnected(bool , bool, QIODevice*) {
    scene->clear();
    scene->addText("Connected. Waiting for camera image ...");
}

void TinaCameraFrontend::onDisconnected(bool ) {
    scene->clear();
    scene->addText("Disconnected");
}

void TinaCameraFrontend::clear(void) {
    scene->clear();
}

// needed for the interface
void TinaCameraFrontend::writeData(QByteArray data) {
    (void)data;
}

void TinaCameraFrontend::updateImage(const QImage image)
{
    current_image = image;
    updateImageScaling();
}

void TinaCameraFrontend::updateImageScaling(void)
{
    scene->clear();
    QImage ni = current_image.scaledToWidth(view->width());
    scene->addPixmap(QPixmap::fromImage(ni));
    scene->setSceneRect(ni.rect());
    view->fitInView(ni.rect(), Qt::KeepAspectRatio);
}

void TinaCameraFrontend::handleButtonDumpOne(void)
{
    qDebug() << "dump one button pressed";
    emit dataReady("\r\ndump_one\r\n");
}

void TinaCameraFrontend::handleButtonDumpAll(bool checked)
{
    qDebug() << "dump all button pressed" << checked;
    emit dataReady(QString("\r\ndump_all %1\r\n").arg(checked).toUtf8());
}

void TinaCameraFrontend::handleButtonFarbsensorInfo(void)
{
    qDebug() << "handleButtonFarbsensorInfo pressed";
    emit dataReady("\r\nroi d\r\n");
    emit dataReady("color d\r\n");
}

void TinaCameraFrontend::handleButtonROIs(bool checked)
{
    qDebug() << "dhandleButtonROIs pressed" << checked;
    emit dataReady(QString("\r\nfarbsensor_display %1\r\n").arg(checked).toUtf8());
}
