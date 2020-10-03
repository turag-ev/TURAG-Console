#ifndef TURAG_CONSOLE_CONSOLE_LIBS_KEYBOARDENABLEDCOMBOBOX_H
#define TURAG_CONSOLE_CONSOLE_LIBS_KEYBOARDENABLEDCOMBOBOX_H

#include <QComboBox>

class KeyboardEnabledComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit KeyboardEnabledComboBox(QWidget *parent = 0);

signals:
    void keyPressed(QKeyEvent* e);
    void keyReleased(QKeyEvent* e);

public slots:

protected:
    virtual void keyPressEvent (QKeyEvent* e);
    virtual void keyReleaseEvent (QKeyEvent* e);

};

#endif // TURAG_CONSOLE_CONSOLE_LIBS_KEYBOARDENABLEDCOMBOBOX_H
