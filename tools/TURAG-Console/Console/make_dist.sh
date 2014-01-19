#!/bin/bash

# aktuelles Verzeichnis speichern
OLD_PWD=`pwd`

# in Verzeichnis von Skript gehen
cd "$( dirname "${BASH_SOURCE[0]}" )"

# Links zu tina und Debug-Server erstellen
ln -s ../../tina ../
ln -s ../Debug-Server/Debug_Server ../

# Links verwenden in Projektdatei
# Backup von Console.pro
cp Console.pro Console.pro.bak
# ../../../tina -> ../tina
sed 's/\.\.\/\.\.\/\.\.\/tina/\.\.\/tina/' Console.pro > Console.new.pro
# ../../Debug-Server -> ..
sed 's/\.\.\/\.\.\/Debug-Server/\.\./' Console.new.pro > Console.pro
rm Console.new.pro

# Konfigurieren mit git-Versionsnummer
qmake EXT_VERSION=~git`date +%Y%m%d`

# Tarball erstellen
make distr

# Tarball in aktuelles Verzeichnis kopieren
echo mv *.tar.gz "$OLD_PWD/"
mv *.tar.gz "$OLD_PWD/"

# Aufräumen
make distclean
rm ../tina ../Debug_Server
rm Console.pro
mv Console.pro.bak Console.pro
