#include "qlineedit2.h"


void QLineEdit2::focusInEvent(QFocusEvent *e) {
    QLineEdit::focusInEvent(e);
    selectAll();
    _selectOnMousePress = true;
}

void QLineEdit2::mousePressEvent(QMouseEvent *me) {
    QLineEdit::mousePressEvent(me);
    if(_selectOnMousePress) {
        selectAll();
        _selectOnMousePress = false;
    }
}
