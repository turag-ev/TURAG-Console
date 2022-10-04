#ifndef RECENT_H
#define RECENT_H

#include <QString>
#include <QSettings>
#include <vector>


class RecentFiles
{
public:
    RecentFiles();

    int maxNumber() const { return max_number_; }
    void setMaxNumber(int value);

    void loadFromSettings(const QString& name, const QSettings& settings = QSettings());
    void saveToSettings(const QString& name, QSettings& settings);
    void saveToSettings(const QString& name)
    {
        QSettings settings;
        saveToSettings(name, settings);
    }

    void add(const QString& file);
    void remove(const QString& file);
    void clear();

	bool isEmpty() const { return files_.empty(); }
	int getSize() const { return files_.size(); }
    const QString& operator[](std::size_t i) const { return files_.at(i); }

    const std::vector<QString>& getFiles() const { return files_; }

private:
    int max_number_ = 5;
    std::vector<QString> files_;

    void correctLength();
};


#endif // RECENT_H
