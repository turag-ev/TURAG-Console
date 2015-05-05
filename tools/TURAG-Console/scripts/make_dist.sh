#!/bin/bash

# aktuelles Verzeichnis speichern
OLD_PWD=`pwd`

if [ "$1" == "" ]; then
	LSB="$(lsb_release -sc)"
else
  LSB="$1"
fi

# in Verzeichnis von Quellcode gehen
cd "$( dirname "${BASH_SOURCE[0]}" )/../Console"

# Links zu tina und Debug-Server erstellen
ln -s -T ../../tina ../tina
ln -s -T ../Debug-Server/Debug_Server ../Debug_Server

mkdir ../libs
ln -s -T ../../../libs/qt ../libs/qt

# Links verwenden in Projektdatei
# Backup von Console.pro
cp Console.pro Console.pro.bak
# ../../../tina -> ../tina
sed -i 's#\.\./\.\./\.\./tina#\.\./tina#' Console.pro
# ../../Debug-Server -> ..
sed -i 's#\.\./\.\./Debug-Server#\.\.#' Console.pro
# ../../../libs/qt -> ../libs/qt
sed -i 's#\.\./\.\./\.\./libs/qt#\.\./libs/qt#' Console.pro

# Konfigurieren mit git-Versionsnummer
qmake EXT_VERSION=~git`date +%Y%m%d`~$LSB

# Tarball erstellen
make distr

# Tarball in aktuelles Verzeichnis kopieren
echo mv *.tar.gz "$OLD_PWD/"
mv *.tar.gz "$OLD_PWD/"

# Aufräumen
make distclean
rm -r ../tina ../Debug_Server ../libs
rm Console.pro
mv Console.pro.bak Console.pro
