#include "connectionwidgetfile.h"
#include "backend/filebackend.h"
#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QStringList>


ConnectionWidgetFile::ConnectionWidgetFile(QWidget *parent) :
    ConnectionWidget("Letzte Dateien", parent)
{
    setObjectName("Datei");

    // button to open file
    file_button = new QPushButton(QIcon::fromTheme("document-open"), "Datei öffnen");

    // vertical layout including the button and a label
    layout->addWidget(file_button, 0, Qt::AlignLeft);

    layout->addSpacing(10);
    addRecentConnections();

    connect(file_button, SIGNAL(clicked()), this, SLOT(onOpenFile()));

}


void ConnectionWidgetFile::onOpenFile() {
    QFileDialog dialog(this, "Logdatei auswählen", "");
    dialog.setFileMode(QFileDialog::ExistingFile);
    if (dialog.exec() == QDialog::Accepted) {
      QStringList files = dialog.selectedFiles();

      QString connectionString = FileBackend::connectionPrefix + files[0];

      bool save = false;
      emit connectionChanged(connectionString, &save, nullptr);
      if (save) {
          saveConnection(connectionString);
          addRecentConnections();
      }
    }

}
