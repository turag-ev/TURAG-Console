#ifndef KEYBOARDENABLEDCOMBOBOX_H
#define KEYBOARDENABLEDCOMBOBOX_H

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

#endif // KEYBOARDENABLEDCOMBOBOX_H
