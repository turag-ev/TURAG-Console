#include <cstdio>
#include <cstdarg>
//#include <QDebug>


extern "C"
void debug_printf(const char* fmt, ...) {
//    char buffer[4096];

//    va_list args;
//    va_start(args, fmt);
//    vsnprintf(buffer, sizeof(buffer), fmt, args);
//    va_end(args);

//    qDebug() << buffer;
}

extern "C"
void debug_puts(const char* s) {
//    qDebug() << s;
}

