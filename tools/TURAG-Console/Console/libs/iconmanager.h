#ifndef ICONMANAGER_H
#define ICONMANAGER_H

#include <QIcon>
#include <QHash>

class IconManager
{
public:
    static void setFallback(const QString& path, const QString& fileExtension) {
        fallbackPath = path;
        fallbackFileExtension = fileExtension;
    }

    static QIcon get(const QString& name) {
        if (!data.contains(name)) {
            if (!fallbackPath.isEmpty()) {
                data.insert(name, QIcon::fromTheme(name, QIcon(fallbackPath + "/" + name + "." + fallbackFileExtension)));
            } else {
                data.insert(name, QIcon::fromTheme(name));
            }
        }

        return data.value(name, QIcon());
    }

private:
    static QHash<const QString, QIcon> data;
    static QString fallbackPath;
    static QString fallbackFileExtension;
};

#endif // ICONMANAGER_H
