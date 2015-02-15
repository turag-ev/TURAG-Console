#include <QCoreApplication>
#include <signal_handler/signalhandler.h>
#include <iostream>
#include <QSerialPort>
#include "feldbusdevice.h"
#include <slave/feldbus.h>
#include <QMetaObject>

using namespace std;

FeldbusDevice* dev;


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    cout << "Hello." << endl;

    SignalHandler::init();
    QObject::connect(SignalHandler::get(), SIGNAL(sigInt()), &a, SLOT(quit()));
    QObject::connect(SignalHandler::get(), SIGNAL(sigKill()), &a, SLOT(quit()));
    QObject::connect(SignalHandler::get(), SIGNAL(sigTerm()), &a, SLOT(quit()));


    if (argc < 2) {
        cout << "no port supplied" << endl;
        exit(1);
    }

    dev = new FeldbusDevice;

    if (dev->init(argv[1])) {
        return a.exec();
    }
}


FeldbusDevice::FeldbusDevice(QObject* parent) :
    QObject(parent), port(nullptr)
{
}

bool FeldbusDevice::init(QString portString) {
    port = new QSerialPort(portString);
    bool success = port->open(QIODevice::ReadWrite);
    if (!success) {
        cout << "Fehler beim Oeffnen der seriellen Konsole: " << port->errorString().toLatin1().constData() << endl;
        return false;
    }

    success = port->setBaudRate(BAUDRATE);
    if (!success) {
        cout << "Fehler beim Setzen der Baudrate: " << port->errorString().toLatin1().constData() << endl;
        return false;
    }

    success = port->setDataBits(QSerialPort::Data8);
    if (!success) {
        cout << "Fehler beim Setzen der Framelaenge: " << port->errorString().toLatin1().constData() << endl;
        return false;
    }

    success = port->setParity(QSerialPort::NoParity);
    if (!success) {
        cout << "Fehler beim Setzen der Paritaet: " << port->errorString().toLatin1().constData() << endl;
        return false;
    }

    success = port->setStopBits(QSerialPort::OneStop);
    if (!success) {
        cout << "Fehler beim Setzen der Stopp-Bits: " << port->errorString().toLatin1().constData() << endl;
        return false;
    }

    success = port->setFlowControl(QSerialPort::NoFlowControl);
    if (!success) {
        cout << "Fehler beim Setzen der Flow-Control: " << port->errorString().toLatin1().constData() << endl;
        return false;
    }

    cout << "Opened port: " << portString.toLatin1().constData() << endl;

    connect(port, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    connect(port, SIGNAL(bytesWritten(qint64)), this, SLOT(dataSend(qint64)));

    turag_feldbus_slave_init();

    connect(&uptimeCounter, &QTimer::timeout, [&](void){turag_feldbus_slave_increase_uptime_counter();});
    uptimeCounter.start(10);

    connect(&processingTimer, &QTimer::timeout, [&](void){turag_feldbus_do_processing();});
    processingTimer.start(0);

    return true;
}

FeldbusDevice::~FeldbusDevice(void) {
    if (port) {
        port->close();
        delete port;
    }

    cout << "Goodbye." << endl;
}


extern "C" void turag_feldbus_hardware_init() {
}
extern "C" void turag_feldbus_slave_rts_off(void) {
}
extern "C" void turag_feldbus_slave_rts_on(void) {
}
extern "C" void turag_feldbus_slave_activate_dre_interrupt(void) {
    dev->sendBuffer.clear();
    dev->generateDataRegisterEmptyInterrupt = true;
    QMetaObject::invokeMethod(dev, "dataRegisterEmpty", Qt::QueuedConnection);
}
extern "C" void turag_feldbus_slave_deactivate_dre_interrupt(void) {
    dev->generateDataRegisterEmptyInterrupt = false;
}
extern "C" void turag_feldbus_slave_activate_rx_interrupt(void) {
}
extern "C" void turag_feldbus_slave_deactivate_rx_interrupt(void) {
}
extern "C" void turag_feldbus_slave_activate_tx_interrupt(void) {
    dev->port->write(dev->sendBuffer);
}
extern "C" void turag_feldbus_slave_deactivate_tx_interrupt(void) {
}
extern "C" void turag_feldbus_slave_start_receive_timeout (void) {
}
extern "C" void turag_feldbus_slave_transmit_byte (uint8_t byte) {
    dev->sendBuffer.append(static_cast<char>(byte));
    QMetaObject::invokeMethod(dev, "dataRegisterEmpty", Qt::QueuedConnection);
}
extern "C" void turag_feldbus_slave_begin_interrupt_protect(void) {
}
extern "C" void turag_feldbus_slave_end_interrupt_protect(void) {
}

void FeldbusDevice::dataReceived(void) {
    QByteArray data(port->readAll());

    cout << "Recv: ";

    for (char byte : data) {
        turag_feldbus_slave_byte_received(byte);
        cout << QString("%1 ").arg(static_cast<uint8_t>(byte)).toLatin1().constData();
    }
    cout << endl;

    turag_feldbus_slave_receive_timeout_occured();
}

void FeldbusDevice::dataSend(qint64) {
    turag_feldbus_slave_transmission_complete();
}

void FeldbusDevice::dataRegisterEmpty(void) {
    if (generateDataRegisterEmptyInterrupt) {
        turag_feldbus_slave_ready_to_transmit();
    }
}

extern "C" FeldbusSize_t turag_feldbus_slave_process_package(uint8_t*, FeldbusSize_t, uint8_t*) {
    return TURAG_FELDBUS_IGNORE_PACKAGE;
}
