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

cp $1 /etc/sysconfig/network-scripts/ifcfg-bond0
cp ifcfg-eno49 /etc/sysconfig/network-scripts/
cp ifcfg-eno50 /etc/sysconfig/network-scripts/

echo "now execute the command: \n> sudo systemctl restart network"
echo "probably better to \n> sudo reboot"
