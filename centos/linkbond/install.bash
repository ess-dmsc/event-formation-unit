#!/bin/bash

if [[ $1 == "" ]]; then
  echo "usage: install.sh [test|parallel|fast]"
  echo 
  echo "installs ifcfg-bond ifcfg-en049 and ifcfg-eno50 files"
  exit 1
fi

function errexit() {
  echo "ERROR: $1"
  exit 1
}

SCRIPTPATH=/etc/sysconfig/network-scripts/

test -e ifcfg-bond0.$1 || errexit "$1 is not a valid target. Please specify one of [test, fast, parallel]"

FILES="ifcfg-bond0 ifcfg-eno49 ifcfg-eno50"
for file in $FILES
do
    cp ${file}.$1 ${SCRIPTPATH}${file}$ || errexit "unable to copy file: $file"
done

echo "now execute the command:"
echo "> sudo systemctl restart network"
echo "if this does not work then"
echo "> sudo reboot"
