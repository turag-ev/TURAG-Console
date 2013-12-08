#include "plaintextfrontend.h"
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

PlainTextFrontend::PlainTextFrontend(QWidget *parent) :
    BaseFrontend("Standard-Konsole", parent), scroll_on_output(true), hasSequentialConnection(false)
{
    QVBoxLayout* layout = new QVBoxLayout();

    textbox = new QTextEdit(this);
    textbox->setReadOnly(true);
    textbox->setWordWrapMode(QTextOption::NoWrap);
    textbox->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    textbox->setFocusPolicy(Qt::NoFocus);
    textbox->setContextMenuPolicy(Qt::NoContextMenu);
    textbox->setFont(QFont("Monospace", 9));

    layout->addWidget(textbox);
	layout->setMargin(0);
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
    connect(white_on_black_action, SIGNAL(triggered()), this, SLOT(onStyleWhiteOnBlack()));

    green_on_black_action = new QAction("Grün auf schwarz", this);
    green_on_black_action->setActionGroup(styleOptions);
    green_on_black_action->setCheckable(true);
    connect(green_on_black_action, SIGNAL(triggered()), this, SLOT(onStyleGreenOnBlack()));

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
}

PlainTextFrontend::~PlainTextFrontend() {
    writeSettings();
}


void PlainTextFrontend::onStyleBlackOnWhite() {
    setStyle(STYLE::BLACK_ON_WHITE);
}

void PlainTextFrontend::onStyleWhiteOnBlack() {
    setStyle(STYLE::WHITE_ON_BLACK);
}

void PlainTextFrontend::onStyleGreenOnBlack() {
    setStyle(STYLE::GREEN_ON_BLACK);
}

void PlainTextFrontend::setStyle(STYLE style) {
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

    case STYLE::WHITE_ON_BLACK:
        p.setColor(QPalette::Base, Qt::black);
        p.setColor(QPalette::Text, QColor(178,178,178));
        break;
    }

    textbox->setPalette(p);
}


void PlainTextFrontend::writeData(QByteArray data) {
    QScrollBar* scrollbar = textbox->verticalScrollBar();
    int value = scrollbar->value();
    bool scroll_to_max = scroll_on_output && value == scrollbar->maximum();

    // sequential devices do always output their contents completely, so clear contents beforehand
    if (!hasSequentialConnection) clear();


    QTextCursor end = textbox->textCursor();
    end.movePosition(QTextCursor::End);

    // the following code is supposed to improve performance for very long strings
    // without newlines. Unfortunately it reduces performance in the opposite case
    // and has some funny side effects.

//    if (!auto_wrap) {
        end.insertText(data);
//    } else {
//        for (auto character : data) {
//            end.insertText(QString(character));

//            if (textbox->cursorRect().x() + textbox->verticalScrollBar()->width() > textbox->maximumViewportSize().width()) {
//                end.deletePreviousChar();
//                end.insertText("\n");
//                end.insertText(QString(character));
//            }
//        }
//    }

    scrollbar->setValue(scroll_to_max ? scrollbar->maximum() : value);
}


void PlainTextFrontend::keyPressEvent ( QKeyEvent * e ) {
  if (e->count() > 0) {
    emit dataReady(e->text().toUtf8());
  } else {
      BaseFrontend::keyPressEvent(e);
  }
}


void PlainTextFrontend::clear(void) {
    textbox->clear();
}

void PlainTextFrontend::setScrollOnOutput(bool on) {
    scroll_on_output = on;
}

void PlainTextFrontend::setAutoWrap(bool on) {
    auto_wrap = on;

    if (on) {
        textbox->setWordWrapMode(QTextOption::WrapAnywhere);
    } else {
        textbox->setWordWrapMode(QTextOption::NoWrap);
    }
}


bool PlainTextFrontend::saveOutput(void) {
    QString filename = QFileDialog::getSaveFileName(this);

    if (filename.isEmpty()) {
        return false;
    }

	QFile savefile(std::move(filename));

	if (!savefile.open(QIODevice::WriteOnly)) {
		return false;
	}

	if (!savefile.isWritable()) {
        return false;
    }

	if (savefile.write(textbox->toPlainText().toUtf8()) == -1) {
        return false;
    }

    return true;
}

void PlainTextFrontend::onConnected(bool readOnly, bool isSequential, QIODevice* dev) {
	Q_UNUSED(dev);

    if (readOnly) {
        paste_action->setEnabled(false);
    } else {
        paste_action->setEnabled(true);
    }

    if (isSequential) {
        clear_action->setEnabled(true);
    } else {
        clear_action->setEnabled(false);
    }

    hasSequentialConnection = isSequential;
}


void PlainTextFrontend::onDisconnected(bool reconnecting) {
    (void) reconnecting;
    // TODO
}

void PlainTextFrontend::onPaste() {
    // TODO
}

void PlainTextFrontend::readSettings() {
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
    case STYLE::WHITE_ON_BLACK:
        white_on_black_action->setChecked(true); break;
    }
}

void PlainTextFrontend::writeSettings() {
    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("scrollOnOutput", scroll_action->isChecked());
    settings.setValue("autoWrap", wrap_action->isChecked());
    settings.setValue("Style", (int)selectedStyle);
}
