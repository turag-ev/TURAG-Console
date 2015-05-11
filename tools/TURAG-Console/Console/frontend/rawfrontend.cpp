#include "rawfrontend.h"
#include <QVBoxLayout>
#include <QScrollBar>
#include <QTextCursor>
#include <QKeyEvent>
#include <QPalette>
#include <QFileDialog>
#include <QAction>
#include <QSettings>
#include <QActionGroup>
#include <QMenu>
#include <QDebug>
#include <QPlainTextEdit>
#include <QApplication>
#include <QClipboard>

RawFrontend::RawFrontend(QWidget *parent) :
	BaseFrontend("Raw-Frontend", parent), scroll_on_output(true), rawBuffer_(new QByteArray)
{
    QVBoxLayout* layout = new QVBoxLayout();

    textbox = new QPlainTextEdit(this);
    textbox->setReadOnly(true);
    textbox->setWordWrapMode(QTextOption::NoWrap);
    textbox->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    textbox->setFocusPolicy(Qt::NoFocus);

    // this option effectively leaves the context menu handling to the BaseFrontend.
    textbox->setContextMenuPolicy(Qt::NoContextMenu);

	QFont font("Monospace", 9);
	font.setStyleHint(QFont::Monospace);
	textbox->setFont(font);

	layout->addWidget(textbox);
	setLayout(layout);

    setFocusPolicy(Qt::WheelFocus);

    // add some action for this frontend
    QAction* separator_action = new QAction(this);
    separator_action->setSeparator(true);
    addAction(separator_action);


    QActionGroup* styleOptions = new QActionGroup(this);

    black_on_white_action = new QAction("Schwarz auf weiß", this);
    black_on_white_action->setActionGroup(styleOptions);
    black_on_white_action->setCheckable(true);
    connect(black_on_white_action, SIGNAL(triggered()), this, SLOT(onStyleBlackOnWhite()));

    white_on_black_action = new QAction("Grau auf schwarz", this);
    white_on_black_action->setActionGroup(styleOptions);
    white_on_black_action->setCheckable(true);
    white_on_black_action->setChecked(true);
    connect(white_on_black_action, SIGNAL(triggered()), this, SLOT(onStyleGreyOnBlack()));

    green_on_black_action = new QAction("Grün auf schwarz", this);
    green_on_black_action->setActionGroup(styleOptions);
    green_on_black_action->setCheckable(true);
    connect(green_on_black_action, SIGNAL(triggered()), this, SLOT(onStyleGreenOnBlack()));

    blue_on_black_action = new QAction("Blau auf schwarz", this);
    blue_on_black_action->setActionGroup(styleOptions);
    blue_on_black_action->setCheckable(true);
    connect(blue_on_black_action, SIGNAL(triggered()), this, SLOT(onStyleBlueOnBlack()));

    raspberry_on_black_action = new QAction("Himbeer auf schwarz", this);
    raspberry_on_black_action->setActionGroup(styleOptions);
    raspberry_on_black_action->setCheckable(true);
    connect(raspberry_on_black_action, SIGNAL(triggered()), this, SLOT(onStyleRaspberryOnBlack()));

    QAction* colorscheme_action = new QAction("Farbschema", this);
    QMenu* colorscheme_menu = new QMenu(this);
    colorscheme_menu->addActions(styleOptions->actions());
    colorscheme_action->setMenu(colorscheme_menu);
    addAction(colorscheme_action);

    QAction* selectall_action = new QAction("Alles markieren", this);
    selectall_action->setShortcut(QKeySequence::SelectAll);
    connect(selectall_action,SIGNAL(triggered()),textbox,SLOT(selectAll()));
    addAction(selectall_action);

    QAction* copy_action = new QAction("Kopieren", this);
    copy_action->setShortcut(QKeySequence::Copy);
    connect(copy_action,SIGNAL(triggered()),textbox,SLOT(copy()));
    addAction(copy_action);

    paste_action = new QAction("Einfügen", this);
    paste_action->setShortcut(QKeySequence::Paste);
    connect(paste_action,SIGNAL(triggered()),this,SLOT(onPaste()));
    addAction(paste_action);

    clear_action = new QAction("Ausgabe löschen", this);
    connect(clear_action,SIGNAL(triggered()),this,SLOT(clear()));
    addAction(clear_action);

    scroll_action = new QAction("Mit Ausgabe scrollen", this);
    scroll_action->setCheckable(true);
    connect(scroll_action,SIGNAL(triggered(bool)),this,SLOT(setScrollOnOutput(bool)));
    addAction(scroll_action);

    wrap_action = new QAction("Ausgabe automatisch umbrechen", this);
    wrap_action->setCheckable(true);
    connect(wrap_action,SIGNAL(triggered(bool)),this,SLOT(setAutoWrap(bool)));
    addAction(wrap_action);

    readSettings();

    updateTimer.setInterval(100);
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(onUpdate()));
}

RawFrontend::~RawFrontend() {
    writeSettings();
}


void RawFrontend::onStyleBlackOnWhite() {
    setStyle(STYLE::BLACK_ON_WHITE);
}

void RawFrontend::onStyleGreyOnBlack() {
    setStyle(STYLE::GREY_ON_BLACK);
}

void RawFrontend::onStyleGreenOnBlack() {
    setStyle(STYLE::GREEN_ON_BLACK);
}

void RawFrontend::onStyleBlueOnBlack() {
    setStyle(STYLE::BLUE_ON_BLACK);
}

void RawFrontend::onStyleRaspberryOnBlack() {
    setStyle(STYLE::RASPBERRY_ON_BLACK);
}

void RawFrontend::setStyle(STYLE style) {
    selectedStyle = style;

    QPalette p = textbox->palette();

    switch (style) {
    case STYLE::BLACK_ON_WHITE:
        p.setColor(QPalette::Base, Qt::white);
        p.setColor(QPalette::Text, Qt::black);
        break;

    case STYLE::GREEN_ON_BLACK:
        p.setColor(QPalette::Base, Qt::black);
        p.setColor(QPalette::Text, Qt::green);
        break;

    case STYLE::GREY_ON_BLACK:
        p.setColor(QPalette::Base, Qt::black);
        p.setColor(QPalette::Text, QColor(178,178,178));
        break;

    case STYLE::BLUE_ON_BLACK:
        p.setColor(QPalette::Base, Qt::black);
        p.setColor(QPalette::Text, QColor(23,74,240));
        break;

    case STYLE::RASPBERRY_ON_BLACK:
        p.setColor(QPalette::Base, Qt::black);
        p.setColor(QPalette::Text, QColor(0xcc,0x00,0x99));
        break;
    }

    textbox->setPalette(p);
}


void RawFrontend::writeData(QByteArray data) {
	rawBuffer_->append(data);
    if (!updateTimer.isActive()) {
        updateTimer.start();
    }
}

void RawFrontend::onUpdate(void) {
	QString temp;

	if (rawBuffer_->size()) {
        // make sure we insert text at the end
        QTextCursor cursor = textbox->textCursor();
        cursor.movePosition(QTextCursor::End);

		static int count = 0;
		for (char byte : *rawBuffer_) {
			temp.append(QString("%1 ").arg(((long)byte) & 0xff, (int)2, (int)16, (QChar)'0'));
			++count;
			if (count == 20) {
				temp.append("\n");
				count = 0;
			}
		}


        QScrollBar* scrollbar = textbox->verticalScrollBar();
        bool scroll_to_max = scroll_on_output && scrollbar->value() == scrollbar->maximum();

        // insert data
		cursor.insertText(temp);

        // handle auto scroll feature
        if (scroll_to_max) {
            scrollbar->setValue(scrollbar->maximum());
        }

		rawBuffer_->clear();
        updateTimer.stop();
    }
}


void RawFrontend::keyPressEvent ( QKeyEvent * e ) {
//  if (e->count() > 0) {
//      qDebug() << "PTF keyPressEvent: '" << e->text().toUtf8() << "'";
//      emit dataReady(e->text().toUtf8());
//  } else {
//      BaseFrontend::keyPressEvent(e);
//  }
}


void RawFrontend::clear(void) {
    textbox->clear();
}

void RawFrontend::setScrollOnOutput(bool on) {
    scroll_on_output = on;
}

void RawFrontend::setAutoWrap(bool on) {
    auto_wrap = on;

    if (on) {
        textbox->setWordWrapMode(QTextOption::WrapAnywhere);
    } else {
        textbox->setWordWrapMode(QTextOption::NoWrap);
    }
}


void RawFrontend::onConnected(bool readOnly, QIODevice*) {
    if (readOnly) {
        paste_action->setEnabled(false);
    } else {
        paste_action->setEnabled(true);
    }
}


void RawFrontend::onDisconnected(bool reconnecting) {
    (void) reconnecting;
    // TODO
}

void RawFrontend::onPaste() {
//    QTextCursor cursor = textbox->textCursor();
//    cursor.movePosition(QTextCursor::End);

//    QClipboard *clipboard = QApplication::clipboard();
//    QString txt = clipboard->text();

//    qDebug() << "PTF paste clipboard: '" << txt.toUtf8() << "'";
//    emit dataReady(txt.toUtf8());
}

void RawFrontend::readSettings() {
    QSettings settings;
    settings.beginGroup(objectName());
    scroll_action->setChecked(settings.value("scrollOnOutput", true).toBool());
    setScrollOnOutput(settings.value("scrollOnOutput", true).toBool());
    wrap_action->setChecked(settings.value("autoWrap", true).toBool());
    setAutoWrap(settings.value("autoWrap", true).toBool());
    setStyle((STYLE)settings.value("Style", (int)STYLE::DEFAULT).toInt());

    switch ((STYLE)settings.value("Style", (int)STYLE::DEFAULT).toInt()) {
    case STYLE::BLACK_ON_WHITE:
        black_on_white_action->setChecked(true); break;
    case STYLE::GREEN_ON_BLACK:
        green_on_black_action->setChecked(true); break;
    case STYLE::GREY_ON_BLACK:
        white_on_black_action->setChecked(true); break;
    case STYLE::BLUE_ON_BLACK:
        blue_on_black_action->setChecked(true); break;
    case STYLE::RASPBERRY_ON_BLACK:
        raspberry_on_black_action->setChecked(true); break;
    }
}

void RawFrontend::writeSettings() {
    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("scrollOnOutput", scroll_action->isChecked());
    settings.setValue("autoWrap", wrap_action->isChecked());
    settings.setValue("Style", (int)selectedStyle);
}
