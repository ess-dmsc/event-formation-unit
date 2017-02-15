#!/bin/bash

IP=$1
PORT=$2

while [[ 1 ]]
do
  INDG=$(cat /proc/net/snmp | grep "Udp:" | grep -v InDatagrams | awk '{print $2}')
  echo "efu2.net.rx_udp $INDG $(date +%s)" | nc $IP $PORT
  sleep 1
done
