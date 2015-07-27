#include <cstdio>
#include <cstdarg>
#include <QDebug>
#include <QString>

extern "C"
void turag_debug_vprintf(const char* fmt, va_list vargs) {
    QString buffer;
	buffer.vsprintf(fmt, vargs);

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

