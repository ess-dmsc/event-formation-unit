#!/bin/bash

function errexit() {
  echo "ERROR: $1"
  exit 1
}

test -e $1 || errexit "invalid file, please specify ifcfg-bond.[test, fast, parallel]"

cp $1 /etc/sysconfig/network-scripts/ifcfg-bond0
cp ifcfg-eno49 /etc/sysconfig/network-scripts/
cp ifcfg-eno50 /etc/sysconfig/network-scripts/

echo "now execute the command: \n> sudo systemctl restart network"
echo "probably better to \n> sudo reboot"
