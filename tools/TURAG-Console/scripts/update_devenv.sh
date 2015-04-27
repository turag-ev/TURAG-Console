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

echo 'EXTRAPACKAGES="apt-transport-https ca-certificates"
OTHERMIRROR="deb [trusted=yes] https://svn.turag.et.tu-dresden.de/debian/ubuntu' $LSB 'main"
APTCACHEHARDLINK=no' > "$HOME/.pbuilderrc"

texec pbuilder-dist $LSB i386 update --release-only
if [ "$(uname -m)" == "x86_64" ]; then
	texec pbuilder-dist $LSB amd64 update --release-only
fi
