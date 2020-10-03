#ifndef TURAG_CONSOLE_CONSOLE_LIBS_ICONMANAGER_H
#define TURAG_CONSOLE_CONSOLE_LIBS_ICONMANAGER_H

#include <QIcon>

class IconManager
{
public:
    static void setFallback(const QString& path, const QString& fileExtension);
    static QIcon get(const QString& name);
	static void clear();
};

#endif // TURAG_CONSOLE_CONSOLE_LIBS_ICONMANAGER_H
