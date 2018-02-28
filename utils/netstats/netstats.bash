#!/bin/bash

IP=$1
PORT=$2

while [[ 1 ]]
do
  DATE=$(date +%s)
  SNMP=$(cat /proc/net/snmp | grep Udp: | grep -v InDatagrams)
  INDG=$(echo $SNMP | awk '{print $2}')
  INERR=$(echo $SNMP | awk '{print $4}')
  echo "efu.net.udp_rx $INDG $DATE"
  echo "efu.net.udp_rxerr $INERR $DATE"
  echo "efu.net.udp_rx $INDG $DATE" | nc $IP $PORT
  echo "efu.net.udp_rxperr $INERR $DATE" | nc $IP $PORT
  sleep 2
done
