#ifndef TURAG_CONSOLE_CONSOLE_LIBS_LINEEDITEXT_H
#define TURAG_CONSOLE_CONSOLE_LIBS_LINEEDITEXT_H

#include <QLineEdit>
#include <QSettings>

class LineEditExt : public QLineEdit
{
    Q_OBJECT
public:
    LineEditExt(
            QString identifier = QString(""),
            QString standardValue = QString(""),
            bool selectAllOnFocus = true,
            QWidget *parent = nullptr) :
        QLineEdit(parent),
        selectOnMousePress_(false),
        selectAllOnFocus_(selectAllOnFocus)
    {

        if (!identifier.isEmpty()) {
            setObjectName(identifier);
            QSettings settings;
            setText(settings.value(identifier, standardValue).toString());
            connect(this, SIGNAL(editingFinished()), this, SLOT(saveContents()));
        } else {
            setText(standardValue);
        }
    }

    virtual ~LineEditExt() {
        if (!objectName().isEmpty()) {
            saveContents();
        }
    }


protected slots:
    void saveContents(void) {
        QSettings settings;
        settings.setValue(objectName(), text());
    }


protected:
    void focusInEvent(QFocusEvent *e);
    void mousePressEvent(QMouseEvent *me);


private:
    // iternal variable to control the selection process
    bool selectOnMousePress_;

    bool saveContents_;
    bool selectAllOnFocus_;

};

#endif // TURAG_CONSOLE_CONSOLE_LIBS_LINEEDITEXT_H
