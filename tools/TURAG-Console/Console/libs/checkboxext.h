#ifndef CHECKBOXEXT_H
#define CHECKBOXEXT_H

#include <QCheckBox>
#include <QSettings>

class CheckBoxExt : public QCheckBox
{
    Q_OBJECT
public:
    explicit CheckBoxExt(
            QString text = QString(""),
            QString identifier = QString(""),
            bool standardValue = false,
            QWidget *parent = 0) :
        QCheckBox(text, parent)
    {
        if (!identifier.isEmpty()) {
            setObjectName(identifier);
            QSettings settings;
            setChecked(settings.value(identifier, standardValue).toBool());
            connect(this, SIGNAL(toggled(bool)), this, SLOT(saveToggled()));
        } else {
            setChecked(standardValue);
        }
    }

    ~CheckBoxExt(void) {
        if (!objectName().isEmpty()) {
            saveToggled();
        }
    }

protected slots:
    void saveToggled(void) {
        QSettings settings;
        settings.setValue(objectName(), isChecked());
    }

};

#endif // CHECKBOXEXT_H
