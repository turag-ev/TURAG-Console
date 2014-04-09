#!/bin/bash

VERBOSE="yes"

# Argumente

if [ "$1" == "" ]; then
	LSB="$(lsb_release -sc)"
else
  LSB="$1"
fi

# Verzeichnisse speichern
WORKING_DIR=`pwd`
ROOT_DIR="$( dirname "${BASH_SOURCE[0]}" )"
SRC_DIR="${ROOT_DIR}/Console"

# Hilfsfunktionen
function techo() {
  echo -e "\e[01;34m$1\e[00m"
} 

function texec() {
  if [ "$VERBOSE" == "yes" ]; then
    echo -e "\e[01;32m$@\e[00m"
  fi

	$@ > /dev/null
	
	OUT=$?
	if [ $OUT -ne 0 ]; then
	  exit $?
	fi
}

# Git-Repro aktuallisieren
cd "${ROOT_DIR}"
techo "Git-Repro aktuallisieren"
texec git pull
cd "${WORKING_DIR}"

# Quellcodepaket erstellen
techo "Quellcodepaket erstellen"
texec "${ROOT_DIR}/make_dist.sh"

SRC_TAR="$(ls -t turag-console-*.tar.gz | head -n 1)"
SRC_VERSION="$(echo ${SRC_TAR} | sed 's/turag-console-\(.*\).tar.gz/\1/')"
DEB_TAR="turag-console_${SRC_VERSION}.orig.tar.gz"

texec mv "${SRC_TAR}" "${DEB_TAR}"
texec tar zxf "${DEB_TAR}"
texec cd "turag-console-${SRC_VERSION}"

texec cp -r "${ROOT_DIR}/debian-${LSB}" debian
texec dch -d "new upstream release" -D "${LSB}"
texec cp -r debian/* "${ROOT_DIR}/debian-${LSB}"

texec debuild -sa -S

