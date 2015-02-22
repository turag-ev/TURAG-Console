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

SRC="$(ls -t *.dsc | head -n 1)"

texec pbuilder-dist "$LSB" i386 build "$SRC"
if [ "$(uname -m)" == "x86_64" ]; then
  texec pbuilder-dist "$LSB" amd64 build "$SRC"
fi
