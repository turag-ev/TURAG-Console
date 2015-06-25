#ifndef ICONMANAGER_H
#define ICONMANAGER_H

#include <QIcon>

class IconManager
{
public:
    static void setFallback(const QString& path, const QString& fileExtension);
    static QIcon get(const QString& name);
	static void clear();
};

#endif // ICONMANAGER_H
