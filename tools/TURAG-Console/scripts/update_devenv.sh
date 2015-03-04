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

texec pbuilder-dist $LSB update --release-only
