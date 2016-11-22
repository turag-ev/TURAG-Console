#include "rawfrontend.h"
#include <libs/iconmanager.h>

#include <tina++/crc.h>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QComboBox>
#include <QDebug>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QSettings>
#include <QTextCursor>
#include <QTextStream>
#include <QVBoxLayout>



RawFrontend::RawFrontend(QWidget *parent) :
	BaseFrontend("Raw-Frontend", IconManager::get("binary-icon"), parent), scroll_on_output(true), rawBuffer_(new QByteArray)
{
    QVBoxLayout* layout = new QVBoxLayout();

	// received text field
	rxText = new QPlainTextEdit(this);
	rxText->setReadOnly(true);
	rxText->setWordWrapMode(QTextOption::NoWrap);
	rxText->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	//rxText->setFocusPolicy(Qt::NoFocus);

    // this option effectively leaves the context menu handling to the BaseFrontend.
	rxText->setContextMenuPolicy(Qt::NoContextMenu);

	QFont font("Monospace", 9);
	font.setStyleHint(QFont::Monospace);
	rxText->setFont(font);


	// sent text field
	txText = new QPlainTextEdit(this);
	txText->setReadOnly(true);
	txText->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	//txText->setFocusPolicy(Qt::NoFocus);

	// this option effectively leaves the context menu handling to the BaseFrontend.
	txText->setContextMenuPolicy(Qt::NoContextMenu);

	txText->setFont(font);


	// input field
	inputText = new QLineEdit;
	connect(inputText, &QLineEdit::returnPressed, this, &RawFrontend::onSendData);
	QLabel* checksumLabel = new QLabel("Checksumme");
	appendChecksum = new QComboBox;
	appendChecksum->addItems(QStringList({"keine", "XOR", "CRC8-Icode"}));
	appendChecksum->setCurrentIndex(0);
	appendChecksum->setToolTip("Automatisch Checksumme ans Ende der Daten hängen");
	sendButton= new QPushButton("Senden");
	connect(sendButton, &QPushButton::clicked, this, &RawFrontend::onSendData);

	QHBoxLayout* hlayout = new QHBoxLayout;
	hlayout->addWidget(inputText);
	hlayout->addWidget(checksumLabel);
	hlayout->addWidget(appendChecksum);
	hlayout->addWidget(sendButton);


	layout->addWidget(rxText);
	layout->addWidget(txText);
	layout->addLayout(hlayout);
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

//	QAction* rxtext_selectall_action = new QAction("Alles markieren", rxText);
//	rxtext_selectall_action->setShortcut(QKeySequence::SelectAll);
//	connect(rxtext_selectall_action,SIGNAL(triggered()),rxText,SLOT(selectAll()));
//	rxText->addAction(rxtext_selectall_action);

//	QAction* rxText_copy_action = new QAction("Kopieren", rxText);
//	rxText_copy_action->setShortcut(QKeySequence::Copy);
//	connect(rxText_copy_action,SIGNAL(triggered()),rxText,SLOT(copy()));
//	rxText->addAction(rxText_copy_action);

//	QAction* txText_copy_action = new QAction("Kopieren", txText);
//	txText_copy_action->setShortcut(QKeySequence::Copy);
//	connect(txText_copy_action,SIGNAL(triggered()),txText,SLOT(copy()));
//	txText->addAction(txText_copy_action);


//	paste_action = new QAction("Einfügen", this);
//    paste_action->setShortcut(QKeySequence::Paste);
//    connect(paste_action,SIGNAL(triggered()),this,SLOT(onPaste()));
	//addAction(paste_action);

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
	//addAction(wrap_action);

    readSettings();

    updateTimer.setInterval(100);
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(onUpdate()));
}

RawFrontend::~RawFrontend() {
    writeSettings();
}


void RawFrontend::onStyleBlackOnWhite() {
    setStyle(Style::BlackOnWhite);
}

void RawFrontend::onStyleGreyOnBlack() {
    setStyle(Style::GrayOnBlack);
}

void RawFrontend::onStyleGreenOnBlack() {
    setStyle(Style::GreenOnBlack);
}

void RawFrontend::onStyleBlueOnBlack() {
    setStyle(Style::BlueOnBlack);
}

void RawFrontend::onStyleRaspberryOnBlack() {
    setStyle(Style::RaspberryOnBlack);
}

void RawFrontend::setStyle(Style style) {
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
	rxText->setStyleSheet(styleSheet);
	txText->setStyleSheet(styleSheet);
}


void RawFrontend::writeData(QByteArray data_) {
	rawBuffer_->append(data_);
    if (!updateTimer.isActive()) {
        updateTimer.start();
    }
}

void RawFrontend::onUpdate(void) {
	QString temp;

	if (rawBuffer_->size()) {
        // make sure we insert text at the end
		QTextCursor cursor = rxText->textCursor();
        cursor.movePosition(QTextCursor::End);

		static int count = 0;
		for (char byte : *rawBuffer_) {
			temp.append(QString("0x%1 ").arg(
                            static_cast<long>(byte) & 0xff,
                            static_cast<int>(2),
                            static_cast<int>(16),
                            static_cast<QChar>('0')));
			++count;
			if (count == 20) {
				temp.append("\n");
				count = 0;
			}
		}


		QScrollBar* scrollbar = rxText->verticalScrollBar();
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

void RawFrontend::onSendData() {
	QString raw_data(inputText->text().trimmed());
	QTextStream in(&raw_data);
//	in.setIntegerBase(16);
	QByteArray out;
	int c;

	// convert input
	while (!in.atEnd()) {
		qint64 pos = in.pos();
		in >> c;
		// check for input which cannot be parsed
		if (in.pos() == pos) {
			return;
		}
		out.append(static_cast<char>(c));
	}
	if (out.size() == 0) return;

	// append checksum
	switch (appendChecksum->currentIndex()) {
	case 1:
		out.append(TURAG::XOR::calculate(out.constData(), out.size()));
		break;

	case 2:
		out.append(TURAG::CRC8::calculate(out.constData(), out.size()));
		break;
	}

	// send data
	emit dataReady(out);


	// display sent data
	QTextCursor cursor = txText->textCursor();
	cursor.movePosition(QTextCursor::End);

	QString temp;
	for (char byte : out) {
		temp.append(QString("0x%1 ").arg(
						static_cast<long>(byte) & 0xff,
						static_cast<int>(2),
						static_cast<int>(16),
						static_cast<QChar>('0')));
	}
	temp.append("\n");

	QScrollBar* scrollbar = rxText->verticalScrollBar();
	bool scroll_to_max = scroll_on_output && scrollbar->value() == scrollbar->maximum();

	// insert data
	cursor.insertText(temp);

	// handle auto scroll feature
	if (scroll_to_max) {
		scrollbar->setValue(scrollbar->maximum());
	}
	inputText->clear();
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
	rxText->clear();
	txText->clear();
}

void RawFrontend::setScrollOnOutput(bool on) {
    scroll_on_output = on;
}

void RawFrontend::setAutoWrap(bool on) {
    auto_wrap = on;

    if (on) {
		rxText->setWordWrapMode(QTextOption::WrapAnywhere);
    } else {
		rxText->setWordWrapMode(QTextOption::NoWrap);
    }
}


void RawFrontend::onConnected(bool readOnly, QIODevice*) {
    if (readOnly) {
		//paste_action->setEnabled(false);
    } else {
		//paste_action->setEnabled(true);
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

void RawFrontend::writeSettings() {
    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("scrollOnOutput", scroll_action->isChecked());
    settings.setValue("autoWrap", wrap_action->isChecked());
    settings.setValue("Style", static_cast<int>(selectedStyle));
}
