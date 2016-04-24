#include <QCoreApplication>
#include <signal_handler/signalhandler.h>
#include <iostream>
#include <QSerialPort>
#include "feldbusdevice.h"
#include <slave/feldbus_stellantriebe.h>
#include <QMetaObject>

using namespace std;

FeldbusDevice* dev;

int32_t tmp1 = 8, tmp2 = 5;

static feldbus_stellantriebe_command_t command_set[] = {
	{ // RS485_STELLANTRIEBE_KEY_CURRENT_ANGLE
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_NO_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_LONG,
		.factor =  1.0f
	},
	{ // RS485_STELLANTRIEBE_KEY_DESIRED_ANGLE
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_LONG,
		.factor =  1.0f
	},
	{ // RS485_STELLANTRIEBE_KEY_MAX_ANGLE
		.value = &tmp1,
		.write_access = 0,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_NONE_TEXT,
		.factor =  1.0f
	},
	{ // RS485_STELLANTRIEBE_KEY_MIN_ANGLE
		.value = &tmp1,
		.write_access = 0,
		.length = 0,
		.factor =  1.0f
	},
	{ // RS485_STELLANTRIEBE_KEY_CURRENT_VELOCITY
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_NO_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor =  1.0f
	},
	{ // RS485_STELLANTRIEBE_KEY_DESIRED_VELOCITY
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor =  1.0f
	},
	{ // RS485_STELLANTRIEBE_KEY_MAX_VELOCITY
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_LONG,
		.factor =  1.0f
	},
	{ // RS485_STELLANTRIEBE_KEY_CURRENT_CURRENT
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_NO_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor =  1.0f
	},
	{ // RS485_STELLANTRIEBE_KEY_DESIRED_CURRENT
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_NONE_TEXT,
		.factor =  1.0f
	},
	{ // RS485_STELLANTRIEBE_KEY_MAX_CURRENT
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor =  1.0f
	},
	{ // RS485_STELLANTRIEBE_KEY_CURRENT_PWM
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_NO_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor =  1.0f
	},
	{ // RS485_STELLANTRIEBE_KEY_DESIRED_PWM
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor =  1.0f
	},
	{ // RS485_STELLANTRIEBE_KEY_MAX_PWM
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor =  1.0f
	},
	{ // RS485_STELLANTRIEBE_KEY_VOLTAGE
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_NO_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor =  1.0f
	},
	{ // RS485_STELLANTRIEBE_KEY_STATUS
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_NO_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{ // RS485_STELLANTRIEBE_KEY_CONTROL_STATE
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{ // RS485_STELLANTRIEBE_DC_KEY_SWITCH_STATUS
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_NO_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{ // RS485_STELLANTRIEBE_DC_KEY_RETURN_TO_HOME
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor =  1.0f
	},
	{
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_NO_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_NO_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_SHORT,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},
	{
		.value = &tmp1,
		.write_access = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_ACCESS_WRITE_ACCESS,
		.length = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_LENGTH_CHAR,
		.factor = TURAG_FELDBUS_STELLANTRIEBE_COMMAND_FACTOR_CONTROL_VALUE
	},

};

const char currentAngle_string[]   = "current Angle[mrad]";
const char desiredAngle_string[]   = "desired Angle[mrad]";
const char maxAngle_string[]   = "Velocities:";
const char minAngle_string[]   = "min Angle[mrad]";
const char currentVelocity_string[]   = "current Velocity[mrad/s]";
const char desiredVelocity_string[]   = "desired Velocity[mrad/s]";
const char maxVelocity_string[]   = "max Velocity[mrad/s]";
const char currentCurrent_string[]   = " ";
const char desiredCurrent_string[]   = " ";
const char maxCurrent_string[]   = "max Current[mA]";
const char currentPwm_string[]   = "current PWM[%]";
const char desiredPwm_string[]   = "desired PWM[%]";
const char maxPwm_string[]   = "max PWM[%]";
const char voltage_string[]   = "voltage[mV]";
const char status_string[]   = "status";
const char controlstate_string[]   = "control state";
const char switch_string[]   = "switch status";
const char home_string[]   = "drive home";
const char curP_string[]   = "current P";
const char curI_string[]   = "current I";
const char current_gain_p_string[]   = "current gain P";
const char current_gain_i_string[]   = "current gain I";
const char velocity_gain_p_inv_string[]   = "velocity_gain_p_inv";
const char velocity_gain_i_inv_string[]   = "velocity_gain_i_inv";
const char position_gain_p_string[]   = "position_gain_p";
const char position_gain_p_inv_string[]   = "position_gain_p_inv";
const char position_gain_i_string[]   = "position_gain_i";
const char position_gain_i_inv_string[]   = "position_gain_i_inv";
const char ctrl_pos_flag_samples_required_string[]   = "ctrl_pos_flag_samples_required";
const char ctrl_pos_flag_samples_max_string[]   = "ctrl_pos_flag_samples_max";
const char ctrl_vel_flag_samples_required_string[]   = "ctrl_vel_flag_samples_required";
const char ctrl_vel_flag_samples_max_string[]   = "ctrl_vel_flag_samples_max";
const char ctrl_cur_flag_samples_required_string[]   = "ctrl_cur_flag_samples_required";
const char ctrl_cur_flag_samples_max_string[]   = "ctrl_cur_flag_samples_max";
const char ctrl_error_delay_required_string[]   = "ctrl_error_delay_required";
const char ctrl_error_samples_max_string[]   = "ctrl_error_samples_max";


const char* string_table[] = {
	currentAngle_string,
	desiredAngle_string,
	maxAngle_string,
	minAngle_string,
	currentVelocity_string,
	desiredVelocity_string,
	maxVelocity_string,
	currentCurrent_string,
	desiredCurrent_string,
	maxCurrent_string,
	currentPwm_string,
	desiredPwm_string,
	maxPwm_string,
	voltage_string,
	status_string,
	controlstate_string,
	switch_string,
	home_string,
	// current controller
	curP_string,
	curI_string,
	current_gain_p_string,
	current_gain_i_string,
	// velocity controller
	velocity_gain_p_inv_string,
	velocity_gain_i_inv_string,
	// position controller
	position_gain_p_string,
	position_gain_p_inv_string,
	position_gain_i_string,
	position_gain_i_inv_string,
	// error flags
	ctrl_pos_flag_samples_required_string,
	ctrl_pos_flag_samples_max_string,
	ctrl_vel_flag_samples_required_string,
	ctrl_vel_flag_samples_max_string,
	ctrl_cur_flag_samples_required_string,
	ctrl_cur_flag_samples_max_string,
	ctrl_error_delay_required_string,
	ctrl_error_samples_max_string
};



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
		a.exec();
    }

	delete dev;
	return 0;
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

//    turag_feldbus_slave_init();
	turag_feldbus_stellantriebe_init(command_set, string_table, sizeof(command_set) / sizeof(command_set[0]));

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

extern "C" FeldbusSize_t turag_feldbus_stellantriebe_process_package(uint8_t* message, FeldbusSize_t message_length, uint8_t* response) {
	return 0;
}

extern "C" void turag_feldbus_stellantriebe_value_changed(uint8_t key) {

}
