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
#include <QPlainTextEdit>
#include <QApplication>
#include <QClipboard>
#include <QEvent>
#include <libs/iconmanager.h>

PlainTextFrontend::PlainTextFrontend(QWidget *parent) :
	BaseFrontend("Text-Konsole (Terminal)", IconManager::get("utilities-terminal"), parent), scroll_on_output(true), buffer_(new QByteArray), cleanedBuffer_(new QByteArray)
{
    QVBoxLayout* layout = new QVBoxLayout();

    textbox = new QPlainTextEdit(this);
    textbox->setReadOnly(true);
    textbox->setWordWrapMode(QTextOption::NoWrap);
	textbox->setTextInteractionFlags(Qt::TextSelectableByMouse);
//	textbox->setFocusPolicy(Qt::NoFocus);
	textbox->installEventFilter(this);

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

PlainTextFrontend::~PlainTextFrontend() {
    writeSettings();
}


void PlainTextFrontend::onStyleBlackOnWhite() {
    setStyle(Style::BlackOnWhite);
}

void PlainTextFrontend::onStyleGreyOnBlack() {
    setStyle(Style::GrayOnBlack);
}

void PlainTextFrontend::onStyleGreenOnBlack() {
    setStyle(Style::GreenOnBlack);
}

void PlainTextFrontend::onStyleBlueOnBlack() {
    setStyle(Style::BlueOnBlack);
}

void PlainTextFrontend::onStyleRaspberryOnBlack() {
    setStyle(Style::RaspberryOnBlack);
}

void PlainTextFrontend::setStyle(Style style) {
    selectedStyle = style;

	QString styleSheet;

	switch (style) {
	case Style::BlackOnWhite:
		styleSheet = "QPlainTextEdit {background-color: white; color: black}";
		break;

	case Style::GreenOnBlack:
		styleSheet = "QPlainTextEdit {background-color: black; color: rgb(0,255,0)}";
		break;

	case Style::GrayOnBlack:
		styleSheet = "QPlainTextEdit {background-color: black; color: rgb(178,178,178)}";
		break;

	case Style::BlueOnBlack:
		styleSheet = "QPlainTextEdit {background-color: black; color: rgb(23,74,240)}";
		break;

	case Style::RaspberryOnBlack:
		styleSheet = "QPlainTextEdit {background-color: black; color: rgb(204, 0, 153)}";
		break;
	}
	textbox->setStyleSheet(styleSheet);
}


void PlainTextFrontend::writeData(QByteArray data_) {
	buffer_->append(data_);
    if (!updateTimer.isActive()) {
        updateTimer.start();
    }
}

void PlainTextFrontend::onUpdate(void) {
    if (buffer_->size()) {
        // make sure we insert text at the end
        QTextCursor cursor = textbox->textCursor();
        cursor.movePosition(QTextCursor::End);

        if (cleanedBuffer_->capacity() < buffer_->size()) {
            cleanedBuffer_->reserve(buffer_->size());
        }

        // clean input stream
		const char* data_ = buffer_->constBegin();
		while (data_ != buffer_->constEnd()) {
			if ((*data_ >= 0x20 && *data_ <= 0x7E) || *data_ == '\n' || *data_ == '\r' || *data_ == '\t' || *data_ < 0) {
				// printable characters, utf-8 characters (checked with: *data < 0)
				// and newlines are piped through
				cleanedBuffer_->append(*data_);
			} else if (*data_ == 0x08 || *data_ == 0x7F) {
                // special handling for backspace and delete characters
                if (cleanedBuffer_->size()) {
                    cleanedBuffer_->chop(1);
                } else {
                    cursor.deletePreviousChar();
                }

				// TODO: Carriage return kann nicht so simpel behandelt werden,
				// das klappt nicht bei \r\n
				// Für Bus-Pirate muss außerdem Esc[2K behandelt werden
				// siehe http://ascii-table.com/ansi-escape-sequences-vt-100.php
				// Bleibt die Frage, ob man weiter um QPlainTextEdit drumrumbaut oder auf ein
				// simpleres Widget aufsetzt. Aber ersteres ist wohl das beste. zB bei auftreten von \r
				// aufs nächste Zeichen warten und abhängig davon das richtige tun, das sollte gehen.

				// Edit 23.05.2016: Da ich weiter unten bei Enter nur noch '\r' sende, sollte
				// das in der Anzeige auch für einen Zeilenumbruch sorgen (was QPlainTextEdit auch
				// per default so macht). Daher hab ich was wieder oben ins Kriterium für nicht
				// näher zu behandelnde Zeichen reingenommen.
//			} else if (*data == '\r') {
//				// handling of carriage return: delete line
//				int indexOfNewline = cleanedBuffer_->lastIndexOf('\n');
//				if (indexOfNewline != -1) {
//					cleanedBuffer_->chop(cleanedBuffer_->size() - indexOfNewline - 1);
//				} else {
//					cleanedBuffer_->clear();
//					while (!cursor.atBlockStart()) {
//						cursor.deletePreviousChar();
//					}
//				}
			} else if (*data_ != '\r') {
				// empty place holder for anything else
                cleanedBuffer_->append("�");
            }

			++data_;
        }

        QScrollBar* scrollbar = textbox->verticalScrollBar();
        bool scroll_to_max = scroll_on_output && scrollbar->value() == scrollbar->maximum();

        // insert data
        cursor.insertText(QString::fromUtf8(*cleanedBuffer_));

        // handle auto scroll feature
        if (scroll_to_max) {
            scrollbar->setValue(scrollbar->maximum());
        }

        buffer_->clear();
        cleanedBuffer_->clear();
        updateTimer.stop();
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


void PlainTextFrontend::onConnected(bool readOnly, QIODevice*) {
    if (readOnly) {
        paste_action->setEnabled(false);
    } else {
        paste_action->setEnabled(true);
    }
}


void PlainTextFrontend::onDisconnected(bool reconnecting) {
    (void) reconnecting;
    // TODO
}

void PlainTextFrontend::onPaste() {
    QTextCursor cursor = textbox->textCursor();
    cursor.movePosition(QTextCursor::End);

    QClipboard *clipboard = QApplication::clipboard();
    QString txt = clipboard->text();

    qDebug() << "PTF paste clipboard: '" << txt.toUtf8() << "'";
    emit dataReady(txt.toUtf8());
}

void PlainTextFrontend::readSettings() {
    QSettings settings;
    settings.beginGroup(objectName());
    scroll_action->setChecked(settings.value("scrollOnOutput", true).toBool());
    setScrollOnOutput(settings.value("scrollOnOutput", true).toBool());
    wrap_action->setChecked(settings.value("autoWrap", true).toBool());
    setAutoWrap(settings.value("autoWrap", true).toBool());
    setStyle(static_cast<Style>(settings.value("Style", static_cast<int>(Style::Default)).toInt()));

    switch (static_cast<Style>(settings.value("Style", static_cast<int>(Style::Default)).toInt())) {
    case Style::BlackOnWhite:
        black_on_white_action->setChecked(true); break;
    case Style::GreenOnBlack:
        green_on_black_action->setChecked(true); break;
    case Style::GrayOnBlack:
        white_on_black_action->setChecked(true); break;
    case Style::BlueOnBlack:
        blue_on_black_action->setChecked(true); break;
    case Style::RaspberryOnBlack:
        raspberry_on_black_action->setChecked(true); break;
    }
}

void PlainTextFrontend::writeSettings() {
    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("scrollOnOutput", scroll_action->isChecked());
    settings.setValue("autoWrap", wrap_action->isChecked());
    settings.setValue("Style", static_cast<int>(selectedStyle));
}

bool PlainTextFrontend::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

		switch (keyEvent->key()) {

		// VT100-compatible cursor control sequences.
		// See: http://www.vt100.net/docs/vt100-ug/chapter3.html#S3.1
		case Qt::Key_Up:
			if (keyEvent->modifiers() == Qt::NoModifier) {
				emit dataReady("\x1b[A");
				qDebug() << "Sent Key_Up";
			}
			break;

		case Qt::Key_Down:
			if (keyEvent->modifiers() == Qt::NoModifier) {
				emit dataReady("\x1b[B");
				qDebug() << "Sent Key_Down";
			}
			break;

		case Qt::Key_Left:
			if (keyEvent->modifiers() == Qt::NoModifier) {
				emit dataReady("\x1b[D");
				qDebug() << "Sent Key_Left";
			}
			break;

		case Qt::Key_Right:
			if (keyEvent->modifiers() == Qt::NoModifier) {
				emit dataReady("\x1b[C");
				qDebug() << "Sent Key_Right";
			}
			break;

// Das war hier mal drin, weil ich dachte, dass es logischer wäre wenn die Enter-Taste
// ein Newline (0x0A) sendet, weil das in Linux als Zeilenende benutzt wird. Im oben
// erwähnten VT100-Protokoll wird allerdings per default für die Enter-/Returntaste nur
// ein Carriage Return (0x0D) sendet, so wie Qt es standardmäßig macht und wie es
// scheinbar auch das Testmenü korrekterweise erwartet. Daher muss diese Anpassung weg.
//
//    	Martin Oemus, 22.04.2016

//			// convert carriage return on enter to line feed
//		case Qt::Key_Return:
//		case Qt::Key_Enter:
//			emit dataReady("\x0a");
//			qDebug() << "Sent line feed";
//			break;

		default:
			QByteArray text(keyEvent->text().toUtf8());
			if (text.size() > 0) {
				emit dataReady(text);

				for (char i : text) {
                    qDebug() << "Sent" << static_cast<uint8_t>(i);
				}
			}
			break;
		}
		return true;
	} else {
		// standard event processing
		return QObject::eventFilter(obj, event);
	}
}
