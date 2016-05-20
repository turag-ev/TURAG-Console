#ifndef RawFrontend_H
#define RawFrontend_H

#include "basefrontend.h"
#include <QTimer>

class QPlainTextEdit;

class RawFrontend : public BaseFrontend
{
    Q_OBJECT

public:
    enum class Style {
        BlackOnWhite,
        GrayOnBlack,
        GreenOnBlack,
        BlueOnBlack,
        RaspberryOnBlack,
        Default = Style::GrayOnBlack
    };

public:
	RawFrontend(QWidget *parent = 0);
	~RawFrontend();


public slots:
    virtual void writeData(QByteArray data);
    virtual void clear(void);

    // frontend was connected to stream
	virtual void onConnected(bool readOnly, QIODevice*);

    // frontend was disconnected from stream
    virtual void onDisconnected(bool reconnecting);


    virtual void setScrollOnOutput(bool on);
    virtual void setStyle(Style style);
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
    Style selectedStyle;

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
	QByteArray* rawBuffer_;

};

#endif // RawFrontend_H
