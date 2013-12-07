#ifndef PLAINTEXTFRONTEND_H
#define PLAINTEXTFRONTEND_H

#include "basefrontend.h"
#include <QTextEdit>

class PlainTextFrontend : public BaseFrontend
{
    Q_OBJECT

public:
    enum class STYLE {
        BLACK_ON_WHITE,
        WHITE_ON_BLACK,
        GREEN_ON_BLACK,
        DEFAULT = STYLE::WHITE_ON_BLACK
    };

protected:
    QTextEdit* textbox;
    bool scroll_on_output;
    bool auto_wrap;
    bool hasSequentialConnection;
    STYLE selectedStyle;

    // menu actions
    QAction* clear_action;
    QAction* paste_action;
    QAction* scroll_action;
    QAction* wrap_action;
    QAction* green_on_black_action;
    QAction* white_on_black_action;
    QAction* black_on_white_action;

    virtual void keyPressEvent ( QKeyEvent * e );
    virtual void readSettings();
    virtual void writeSettings();

public:
    PlainTextFrontend(QWidget *parent = 0);
    ~PlainTextFrontend();


protected slots:
    void onStyleBlackOnWhite();
    void onStyleWhiteOnBlack();
    void onStyleGreenOnBlack();

public slots:
    virtual void writeData(QByteArray data);
    virtual void clear(void);

    // frontend was connected to stream
    virtual void onConnected(bool readOnly, bool isSequential, QIODevice*);

    // frontend was disconnected from stream
    virtual void onDisconnected(bool reconnecting);


    virtual bool saveOutput(void);

    virtual void setScrollOnOutput(bool on);
    virtual void setStyle(STYLE style);
    virtual void setAutoWrap(bool on);
    virtual void onPaste();

};

#endif // PLAINTEXTFRONTEND_H
