#!/bin/bash

function texec() {
  echo -e "\e[01;32m$@\e[00m"
					  
  $@
						  
  OUT=$?
  if [ $OUT -ne 0 ]; then
    exit $?
  fi
}

if [ "$1" == "" ]; then
	LSB="$(lsb_release -sc)"
else
  LSB="$1"
fi

# Packete installieren
texec sudo apt-get install -y build-essential debhelper dh-make quilt fakeroot lintian pbuilder debootstrap cdbs ubuntu-dev-tools devscripts

# pbuilderrc setzen
if [ -f "$HOME/.pbuilderrc" ]; then
  mv "$HOME/.pbuilderrc" "$HOME/.pbuilderrc.bak"
fi

echo 'EXTRAPACKAGES="apt-transport-https ca-certificates"
OTHERMIRROR="deb [trusted=yes] https://svn.turag.et.tu-dresden.de/debian/ubuntu $LSB main"
APTCACHEHARDLINK=no' > "$HOME/.pbuilderrc"

# pbuilder Umgebung erstellen
texec pbuilder-dist $LSB create --debootstrapopts --variant=buildd
