#ifndef CHECKACTIONEXT_H
#define CHECKACTIONEXT_H

#include <QAction>
#include <QSettings>

class CheckActionExt : public QAction
{
    Q_OBJECT
public:
    explicit CheckActionExt(const QString& identifier, const QString &text, bool standardValue, QObject* parent) :
        QAction(text, parent)
    {
        setCheckable(true);

        if (!identifier.isEmpty()) {
            setObjectName(identifier);
            QSettings settings;
            setChecked(settings.value(identifier, standardValue).toBool());
            connect(this, SIGNAL(triggered(bool)), this, SLOT(saveToggled()));
        } else {
            setChecked(standardValue);
        }
    }

    ~CheckActionExt(void) {
        if (!objectName().isEmpty()) {
            saveToggled();
        }
    }

signals:

protected slots:
    void saveToggled(void) {
        QSettings settings;
        settings.setValue(objectName(), isChecked());
    }


};

#endif // CHECKACTIONEXT_H
