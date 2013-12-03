#ifndef CMENU_FRONTEND_H
#define CMENU_FRONTEND_H

#include "plaintextfrontend.h"

class TinaInterface;
class QByteArray;
class QWidget;


class CmenuFrontend : public PlainTextFrontend {
    Q_OBJECT

protected:
    TinaInterface* dataInterface;

public:
    explicit CmenuFrontend(QWidget *parent = 0);


public slots:
    virtual void writeData(QByteArray data);

private slots:
    void writeDataBase(QByteArray data);

};

#endif // CMENU_FRONTEND_H
