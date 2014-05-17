#ifndef PLAINTEXTFRONTEND_H
#define PLAINTEXTFRONTEND_H

#include "basefrontend.h"
#include <QTimer>

class QPlainTextEdit;

class PlainTextFrontend : public BaseFrontend
{
    Q_OBJECT

public:
    enum class STYLE {
        BLACK_ON_WHITE,
        GREY_ON_BLACK,
        GREEN_ON_BLACK,
        BLUE_ON_BLACK,
        RASPBERRY_ON_BLACK,
        DEFAULT = STYLE::GREY_ON_BLACK
    };

public:
    PlainTextFrontend(QWidget *parent = 0);
    ~PlainTextFrontend();


public slots:
    virtual void writeData(QByteArray data);
    virtual void clear(void);

    // frontend was connected to stream
    virtual void onConnected(bool readOnly, bool isBuffered, QIODevice*);

    // frontend was disconnected from stream
    virtual void onDisconnected(bool reconnecting);


    virtual void setScrollOnOutput(bool on);
    virtual void setStyle(STYLE style);
    virtual void setAutoWrap(bool on);
    virtual void onPaste();

protected slots:
    void onStyleBlackOnWhite();
    void onStyleGreyOnBlack();
    void onStyleGreenOnBlack();
    void onStyleBlueOnBlack();
    void onStyleRaspberryOnBlack();

    void onUpdate(void);

protected:
    QPlainTextEdit* textbox;
    bool scroll_on_output;
    bool auto_wrap;
    bool hasBufferedConnection;
    STYLE selectedStyle;

    // menu actions
    QAction* clear_action;
    QAction* paste_action;
    QAction* scroll_action;
    QAction* wrap_action;
    QAction* green_on_black_action;
    QAction* raspberry_on_black_action;
    QAction* blue_on_black_action;
    QAction* white_on_black_action;
    QAction* black_on_white_action;

    virtual void keyPressEvent ( QKeyEvent * e );
    virtual void readSettings();
    virtual void writeSettings();

private:
    QTimer updateTimer;
    QByteArray buffer_;

};

#endif // PLAINTEXTFRONTEND_H
