#ifndef TURAG_CONSOLE_CONSOLE_LIBS_COMBOBOXEXT_H
#define TURAG_CONSOLE_CONSOLE_LIBS_COMBOBOXEXT_H

#include <QComboBox>
#include <QSettings>

class ComboBoxExt : public QComboBox
{
    Q_OBJECT
public:
    explicit ComboBoxExt( QString identifier = QString(""),
                          int standardSelectedIndex = 0,
                          QWidget *parent = 0) :
        QComboBox(parent), savedIndex(standardSelectedIndex)
    {
        if (!identifier.isEmpty()) {
            setObjectName(identifier);
            QSettings settings;
            savedIndex = settings.value(identifier, standardSelectedIndex).toInt();
            connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(saveIndex()));
        }
    }

    ~ComboBoxExt(void) {
        if (!objectName().isEmpty()) {
            saveIndex();
        }
    }

    void selectStandardItem(void) {
        setCurrentIndex(savedIndex);
    }

protected slots:
    void saveIndex(void) {
        QSettings settings;
        settings.setValue(objectName(), currentIndex());
    }


private:
    int savedIndex;
};

#endif // TURAG_CONSOLE_CONSOLE_LIBS_COMBOBOXEXT_H
