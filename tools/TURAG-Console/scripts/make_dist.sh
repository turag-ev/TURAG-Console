#!/bin/bash -e

# aktuelles Verzeichnis speichern
OLD_PWD=`pwd`

if [ "$1" == "" ]; then
	LSB="$(lsb_release -sc)"
else
  LSB="$1"
fi

# goto temporary build folder
rm -rf .build-distr
mkdir .build-distr
cd .build-distr

# Konfigurieren mit git-Versionsnummer
SRC="$( dirname "${BASH_SOURCE[0]}" )/../Console"
qmake EXT_VERSION="~git`date +%Y%m%d`~$LSB" "$SRC/Console.pro"

# Tarball erstellen
make distr

# Tarball in aktuelles Verzeichnis kopieren
mv *.tar.bz2 ..

# Aufräumen
cd ..
rm -rf .build-distr
