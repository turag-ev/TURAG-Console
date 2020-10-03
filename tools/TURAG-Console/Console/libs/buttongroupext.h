#ifndef TURAG_CONSOLE_CONSOLE_LIBS_BUTTONGROUPEXT_H
#define TURAG_CONSOLE_CONSOLE_LIBS_BUTTONGROUPEXT_H

#include <QButtonGroup>
#include <QSettings>
#include <QList>
#include <QAbstractButton>

class ButtonGroupExt : public QButtonGroup {
    Q_OBJECT

public:
    ButtonGroupExt(QString identifier, QObject* parent = nullptr) :
        QButtonGroup(parent)
    {
        if (!identifier.isEmpty()) {
            setObjectName(identifier);
            connect(this, SIGNAL(buttonToggled(int,bool)), this, SLOT(saveSettings()));
        }
    }

    void readSettings(void) {
        if (!objectName().isEmpty()) {
            QSettings settings;
            QByteArray checkedMask(settings.value(objectName(), QByteArray()).toByteArray());

            if (checkedMask.size() == buttons().size()) {
                int i = 0;
                for (QAbstractButton* button : buttons()) {
                    button->setChecked(static_cast<bool>(checkedMask.at(i)));
                    ++i;
                }
            }
        }
    }

    ~ButtonGroupExt(void) {
        if (!objectName().isEmpty()) {
            saveSettings();
        }
    }

protected slots:
    void saveSettings(void) {
        QByteArray checkedMask(buttons().size(), 0);
        int i = 0;

        for (QAbstractButton* button : buttons()) {
            if (button->isChecked()) {
                checkedMask.data()[i] = 1;
            }
            ++i;
        }

        QSettings settings;
        settings.setValue(objectName(), checkedMask);
    }
};


#endif // TURAG_CONSOLE_CONSOLE_LIBS_BUTTONGROUPEXT_H

