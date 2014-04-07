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
	PBUILDER_32="~/pbuilder/${LSB}-i386_result"
	PBUILDER_64="~/pbuilder/${LSB}_result"
else
  PBUILDER_32="~/pbuilder/${LSB}_result"
fi

read -p "Bitte zu TURAG-VPN verbinden. Weiter mit ENTER."

# debs hochladen
texec scp "$(ls -t ${PBUILDER_32}/*.deb | head -n 1)" root@foxie:/srv/reprepro/ubuntu/incoming/
if [ "$(uname -m)" == "x86_64" ]; then
	texec scp "$(ls -t ${PBUILDER_64}/*.deb | head -n 1)" root@foxie:/srv/reprepro/ubuntu/incoming/
fi






