#include "connectionwidgetfile.h"
#include "backend/filebackend.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QStringList>
#include <QUrl>
#include <libs/iconmanager.h>


ConnectionWidgetFile::ConnectionWidgetFile(QWidget *parent) :
    ConnectionWidget("Letzte Dateien", parent)
{
    setObjectName("Datei");

    // button to open file
    file_button = new QPushButton(IconManager::get("document-open"), "Datei öffnen");

    // vertical layout including the button and a label
    layout->addWidget(file_button, 0, Qt::AlignLeft);

    addRecentConnections();

    connect(file_button, SIGNAL(clicked()), this, SLOT(onOpenFile()));
}


void ConnectionWidgetFile::onOpenFile() {
    QFileDialog dialog(this, "Logdatei auswählen", "");
    dialog.setFileMode(QFileDialog::ExistingFile);
    if (dialog.exec() == QDialog::Accepted) {
      QStringList files = dialog.selectedFiles();

	  QUrl url;
	  url.setScheme("file");
	  url.setPath(files[0]);

      bool save = false;
	  emit connectionChanged(url.toDisplayString(), &save, nullptr);
      if (save) {
		  saveConnection(url.toDisplayString());
          addRecentConnections();
      }
    }

}
