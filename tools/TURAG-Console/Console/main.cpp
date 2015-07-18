
#include <libs/log.h>
#include <libs/iconmanager.h>
#include "mainwindow.h"

#include <QCoreApplication>
#include <QApplication>
#include <QCommandLineParser>
#include <QLocale>
#include <QStringList>
#include <QDebug>

#define _TO_STRING(x) #x
#define TO_STRING(x) _TO_STRING(x)


int main(int argc, char *argv[]) {
	QCoreApplication::setApplicationName(TO_STRING(PACKAGE_NAME));
	QCoreApplication::setOrganizationName("TURAG");
	QCoreApplication::setOrganizationDomain("turag.de");
	QCoreApplication::setApplicationVersion("v" TO_STRING(PACKAGE_VERSION));

	QLocale curLocale(QLocale("de_DE"));
	QLocale::setDefault(curLocale);
	setlocale(LC_ALL, "de");

	Log::captureQtDebugMessages(true);
	IconManager::setFallback(":/images", "png");


	// set up our options parser with all options we need.
	QCommandLineParser parser;
	parser.setApplicationDescription("TURAG-Console is a Eierlegende-Wollmilchsau.");
	parser.addOptions({
						  /*{"list-backends", "Lists all available backends and the schemes they support for opening."},
						  {"list-frontends", "Lists all available CLI-frontends."},
						  {"frontend", "Opens the supplied URL with the the given <frontend>", "frontend"},
						  {"frontend-args", "Arguments which are redirected to the selected frontend", "arg-string"},
						  {"frontend-help", "Displays the usage info of the specified CLI-frontend.", "frontend"},*/
					  });
	parser.addPositionalArgument("url", "URL to open. If this arguments is not accompanied by any of the options above, the application will run in GUI mode.");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

	// To save ressources and reduce the startup-time in cases where we do not need a GUI
	// we want to create a QCoreApplication rather than a full blown QApplication. To do
	// so, we have to parse the options before creating the required application class instance.
	// Our distinguishing feature is whether the user supplied any of the named options from
	// above.
	QStringList arguments;
	for (int i = 0; i < argc; ++i) {
		arguments.append(argv[i]);
	}
	parser.parse(arguments);

	if (parser.optionNames().count() == 0) {
		qDebug() << "no known command line options given --> starting GUI session";

		QApplication a(argc, argv);
		QObject::connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

		// make qt cute :)
		a.setStyleSheet(
					"QStatusBar { border-top: 1px solid #ddd; }"
					"QStatusBar::item { border: 0px solid black; } ");

		MainWindow w;
		w.show();

		if (arguments.size() > 1) {
			w.openUrl(arguments.at(1));
		}

		return a.exec();

	} else {
		QCoreApplication a(argc, argv);
		parser.process(a);
	}

}

