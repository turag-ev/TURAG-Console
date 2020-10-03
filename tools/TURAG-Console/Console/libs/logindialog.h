#ifndef TURAG_CONSOLE_CONSOLE_LIBS_LOGINDIALOG_H
#define TURAG_CONSOLE_CONSOLE_LIBS_LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>

class QLabel;
class QDialogButtonBox;

class LoginDialog : public QDialog
{
	Q_OBJECT
public:
	LoginDialog();
	~LoginDialog();

	QString getUsername(void) {
		return editUsername->text();
	}

	QString getPassword(void) {
		return editPassword->text();
	}

private:
	QLabel* labelUsername;
	QLabel* labelPassword;
	QLineEdit* editUsername;
	QLineEdit* editPassword;
	QDialogButtonBox* buttons;
};

#endif // TURAG_CONSOLE_CONSOLE_LIBS_LOGINDIALOG_H
