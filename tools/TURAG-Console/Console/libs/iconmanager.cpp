#include "iconmanager.h"

#include <QHash>

namespace {

QHash<const QString, QIcon> data;
QString fallbackPath;
QString fallbackFileExtension;

} // namespace

void IconManager::setFallback(const QString& path, const QString& fileExtension) {
	fallbackPath = path;
	fallbackFileExtension = fileExtension;
}

QIcon IconManager::get(const QString& name) {
	if (!data.contains(name)) {
		if (!fallbackPath.isEmpty()) {
			data.insert(name, QIcon::fromTheme(name, QIcon(fallbackPath + "/" + name + "." + fallbackFileExtension)));
		} else {
			data.insert(name, QIcon::fromTheme(name));
		}
	}

	return data.value(name, QIcon());
}

void IconManager::clear()
{
	data.clear();
}
