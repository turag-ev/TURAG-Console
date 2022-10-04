#include "recent.h"

#include <QSettings>


RecentFiles::RecentFiles()
{

}

void RecentFiles::setMaxNumber(int value)
{
    max_number_ = value;

    correctLength();
}

void RecentFiles::loadFromSettings(const QString &name, const QSettings &settings)
{
    QStringList list = settings.value(name).toStringList();

    files_.clear();
    files_.reserve(list.size());
    std::copy(list.begin(), list.end(), std::back_inserter(files_));
    correctLength();
}

void RecentFiles::saveToSettings(const QString &name, QSettings &settings)
{
    QStringList list;
    list.reserve(files_.size());
    std::copy(files_.begin(), files_.end(), std::back_inserter(list));

    settings.setValue(name, list);
}

void RecentFiles::add(const QString &file)
{
    // remove duplicate
    remove(file);

    // add
    files_.insert(files_.begin(), file);

    // correct length
    correctLength();
}

void RecentFiles::remove(const QString &file)
{
    files_.erase(std::remove(files_.begin(), files_.end(), file), files_.end());
}

void RecentFiles::clear()
{
    files_.clear();
}

void RecentFiles::correctLength()
{
    if (max_number_< 0) return;

    if (files_.size() > std::size_t(max_number_))
    {
        files_.erase(files_.begin() + max_number_, files_.end());
    }
}



