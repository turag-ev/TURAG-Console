#include <cstdio>
#include <cstdarg>
#include <QDebug>


extern "C"
void turag_debug_printf(const char* fmt, ...) {
    char buffer[4096];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    char* pChar = buffer;
    while(*pChar) {
        if (*pChar < 32) *pChar = 32;
        ++pChar;
    }

    qDebug() << buffer;
}

extern "C"
void turag_debug_puts(const char* s) {
    qDebug() << s;
}

