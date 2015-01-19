#include "lineeditext.h"


void LineEditExt::focusInEvent(QFocusEvent *e) {
    QLineEdit::focusInEvent(e);
    selectAll();
    selectOnMousePress_ = true;
}

void LineEditExt::mousePressEvent(QMouseEvent *me) {
    QLineEdit::mousePressEvent(me);
    if (selectOnMousePress_) {
        setSelection(text().count(), -text().count());
        selectOnMousePress_ = false;
    }
}
