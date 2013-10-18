#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QScrollBar>

#include "commandview.h"


CommandView::CommandView(QWidget *parent) :
    BaseFrontend("C-Menu", parent),
    lineInterface(new LineInterface(this)),
    output_(new QTextEdit),
    input_(new QLineEdit),
    scroll_on_output_(false)
{
  output_->setReadOnly(true);

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(output_);
  layout->addWidget(input_);
  layout->setSpacing(2);
  layout->setContentsMargins(2, 2, 2, 2);
  setLayout(layout);

  connect(input_, SIGNAL(textEdited(QString)), this, SLOT(onChar(QString)));

  // pipe incoming data through line interface
  connect(lineInterface, SIGNAL(lineReady(QByteArray)), this, SLOT(writeLine(QByteArray)));
}

void CommandView::writeData(QByteArray data) {
    lineInterface->writeData(data);
}

void CommandView::writeLine(QByteArray data) {
    if (data.size() > 2) {
        char level = data.at(1);
        unsigned source = data.at(0);

        data.remove(0, 2);

        if (level == 'C' && source == '^') {
            buffer_.append(data);
            buffer_.append("<br>");
        }
    }
}

void CommandView::flush() {
  QTextCursor cursor = output_->textCursor();
  QTextCursor backup_cursor = cursor;
  cursor.movePosition(QTextCursor::End);
  output_->setTextCursor(cursor);

  output_->insertHtml(buffer_);

  output_->setTextCursor(backup_cursor);
  buffer_.clear();
}


void CommandView::onChar(QString character) {
  if (character.length() > 0) {
    QByteArray c = character.toUtf8();
    emit dataReady(c);
    input_->clear();
  }
}

void CommandView::setScrollOnOutput(bool on) {
  if (scroll_on_output_ != on) {
    scroll_on_output_ = on;
    if (on) {
      connect(output_, SIGNAL(textChanged()),
              this, SLOT(scroll()));
    } else {
      disconnect(this, SLOT(scroll()));
    }
  }
}

void CommandView::scroll() {
  QScrollBar* scrollbar = output_->verticalScrollBar();

  if (scrollbar->value() == scrollbar->maximum()) {
    scrollbar->setSliderPosition(scrollbar->maximum());
  }
}


void CommandView::clear(void) {
    output_->clear();
}

bool CommandView::saveOutput(void) {
    return false;
}
