#ifndef SPLITTEREXT_H
#define SPLITTEREXT_H

#include <QSplitter>
#include <QSettings>
#include <QDebug>

class SplitterExt : public QSplitter
{
    Q_OBJECT
public:
    explicit SplitterExt(const QString& identifier, QWidget *parent = 0) :
        QSplitter(parent)
    {
        if (!identifier.isEmpty()) {
            setObjectName(identifier);
            connect(this, SIGNAL(splitterMoved(int,int)), this, SLOT(saveState()));
        }
    }

    bool restoreState(void) {
        if (!objectName().isEmpty()) {
            QSettings settings;
            return QSplitter::restoreState(settings.value(objectName()).toByteArray());
        }
        return false;
    }

    ~SplitterExt(void) {
        if (!objectName().isEmpty()) {
            saveState();
        }
    }


signals:

protected slots:
    void saveState(void) {
        QSettings settings;
        settings.setValue(objectName(), QSplitter::saveState());
    }

};

#endif // SPLITTEREXT_H
