#include "signalhandler.h"
#include <signal.h>


void SignalHandler::emitSigKill() {
    emit sigKill();
}


void SignalHandler::emitSigInt() {
    emit sigInt();
}

void SignalHandler::emitSigTerm() {
    emit sigTerm();
}

extern "C" void sigIntHandler (int) {
    if (SignalHandler::get()) {
        SignalHandler::get()->emitSigInt();
    }
}

#ifdef _POSIX
extern "C" void sigKillHandler (int) {
    if (SignalHandler::get()) {
        SignalHandler::get()->emitSigKill();
    }
}
#endif

extern "C" void sigTermHandler (int) {
    if (SignalHandler::get()) {
        SignalHandler::get()->emitSigTerm();
    }
}

SignalHandler* SignalHandler::get(void) {
    static SignalHandler handler;
    return &handler;
}

void SignalHandler::init() {
    SignalHandler::get();
    signal(SIGINT, sigIntHandler);
#ifdef _POSIX
    signal(SIGKILL, sigKillHandler);
#endif
    signal(SIGTERM, sigTermHandler);
}
