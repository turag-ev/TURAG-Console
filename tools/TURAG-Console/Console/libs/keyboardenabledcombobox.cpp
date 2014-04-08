#include "keyboardenabledcombobox.h"

KeyboardEnabledComboBox::KeyboardEnabledComboBox(QWidget *parent) :
    QComboBox(parent)
{
}


void KeyboardEnabledComboBox::keyPressEvent (QKeyEvent* e) {
    emit keyPressed(e);
    QComboBox::keyPressEvent(e);
}

void KeyboardEnabledComboBox::keyReleaseEvent (QKeyEvent* e) {
    emit keyReleased(e);
    QComboBox::keyReleaseEvent(e);
}
