#include "logindialog.h"
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>

LoginDialog::LoginDialog()
{
	QGridLayout* formGridLayout = new QGridLayout( this );

	editUsername = new QLineEdit;
	// initialize the password field so that it does not echo
	// characters
	editPassword = new QLineEdit;
	editPassword->setEchoMode( QLineEdit::Password );

	// initialize the labels
	labelUsername = new QLabel;
	labelPassword = new QLabel;
	labelUsername->setText( tr( "Username" ) );
	labelUsername->setBuddy( editUsername );
	labelPassword->setText( tr( "Password" ) );
	labelPassword->setBuddy( labelPassword );

	// initialize buttons
	buttons = new QDialogButtonBox;
	buttons->addButton( QDialogButtonBox::Ok );
	buttons->addButton( QDialogButtonBox::Cancel );
	buttons->button( QDialogButtonBox::Ok )->setText( tr("Login") );
	buttons->button( QDialogButtonBox::Cancel )->setText( tr("Abort") );

	// connects slots
	connect( buttons->button( QDialogButtonBox::Cancel ), SIGNAL (clicked()), this, SLOT (reject()));
	connect( buttons->button( QDialogButtonBox::Ok ), SIGNAL (clicked()), this, SLOT (accept()));

	// place components into the dialog
	formGridLayout->addWidget( labelUsername, 0, 0 );
	formGridLayout->addWidget( editUsername, 0, 1 );
	formGridLayout->addWidget( labelPassword, 1, 0 );
	formGridLayout->addWidget( editPassword, 1, 1 );
	formGridLayout->addWidget( buttons, 2, 0, 1, 2 );

	setLayout( formGridLayout );}

LoginDialog::~LoginDialog()
{

}

