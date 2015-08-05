#!/bin/bash -e

# aktuelles Verzeichnis speichern
OLD_PWD=`pwd`

if [ "$1" == "" ]; then
	LSB="$(lsb_release -sc)"
else
  LSB="$1"
fi

# in Verzeichnis von Quellcode gehen
cd "$( dirname "${BASH_SOURCE[0]}" )/../Console"

# Konfigurieren mit git-Versionsnummer
qmake EXT_VERSION="~git`date +%Y%m%d`~$LSB"

# Tarball erstellen
make distr

# Tarball in aktuelles Verzeichnis kopieren
mv *.tar.gz "$OLD_PWD/"

# Aufräumen
make distclean
