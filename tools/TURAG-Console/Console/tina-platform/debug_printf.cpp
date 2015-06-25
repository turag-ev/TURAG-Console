#include <cstdio>
#include <cstdarg>
#include <QDebug>
#include <QString>

extern "C"
void turag_debug_printf(const char* fmt, ...) {
    QString buffer;

    va_list args;
    va_start(args, fmt);
    buffer.vsprintf(fmt, args);
    va_end(args);

    /*char* pChar = buffer;
    while(*pChar) {
        if (*pChar < 32) *pChar = 32;
        ++pChar;
    }*/

    qDebug() << buffer.constData();
}

extern "C"
void turag_debug_puts(const char* s)
{
	QString out(s);
    out = out.right(1);
    out.chop(2);
    qDebug() << out.constData();
}

