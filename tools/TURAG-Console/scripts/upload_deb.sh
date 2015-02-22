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

if [ "$(uname -m)" == "x86_64" ]; then
  PBUILDER_32="${HOME}/pbuilder/${LSB}-i386_result"
  PBUILDER_64="${HOME}/pbuilder/${LSB}_result"
else
  PBUILDER_32="${HOME}/pbuilder/${LSB}_result"
fi

# debs hochladen
texec scp "$(ls -t ${PBUILDER_32}/*.deb | head -n 1)" root@foxie:/srv/reprepro/ubuntu/incoming/
if [ "$(uname -m)" == "x86_64" ]; then
  texec scp "$(ls -t ${PBUILDER_64}/*.deb | head -n 1)" root@foxie:/srv/reprepro/ubuntu/incoming/
fi

