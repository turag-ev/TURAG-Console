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
SCRIPT_DIR="$( dirname "${BASH_SOURCE[0]}" )"
ROOT_DIR="${SCRIPT_DIR}/.."
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

# Git-Repro aktualisieren
cd "${ROOT_DIR}"
techo "Git-Repro aktualisieren"
texec git pull
cd "${WORKING_DIR}"

# Quellcodepaket erstellen
techo "Quellcodepaket erstellen"
texec "${SCRIPT_DIR}/make_dist.sh ${LSB}"

SRC_TAR="$(ls -t turag-console-*.tar.bz2 | head -n 1)"
SRC_VERSION="$(echo ${SRC_TAR} | sed 's/turag-console-\(.*\).tar.bz2/\1/')"
DEB_TAR="turag-console_${SRC_VERSION}.orig.tar.bz2"

texec mv "${SRC_TAR}" "${DEB_TAR}"
texec tar -axf "${DEB_TAR}"
texec cd "turag-console-${SRC_VERSION}"

texec cp -r "${ROOT_DIR}/debian-${LSB}" debian
texec dch -d "new upstream release" -D "${LSB}"
texec cp -r debian/* -t "${ROOT_DIR}/debian-${LSB}"

texec debuild -sa -S

