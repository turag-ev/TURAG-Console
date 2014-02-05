#ifndef QLINEEDIT2_H
#define QLINEEDIT2_H

#include <QLineEdit>

class QLineEdit2 : public QLineEdit
{
    Q_OBJECT
public:
    QLineEdit2(QWidget *parent = nullptr) :
        QLineEdit(parent), _selectOnMousePress(false) { }

    virtual ~QLineEdit2() {}

protected:
    void focusInEvent(QFocusEvent *e);
    void mousePressEvent(QMouseEvent *me);

    bool _selectOnMousePress;

};

#endif // QLINEEDIT2_H
