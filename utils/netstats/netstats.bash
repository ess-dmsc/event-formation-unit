#!/bin/bash

# on mjc workstation, try 172.17.12.31 2003 enp0s25

ip=$1
port=$2
dev=${3:-enp0s31f6}

function errexit() {
    echo error: device $1 doesn\'t exist
    exit 1
}


ethtool -S $dev || errexit $dev

while [[ 1 ]]
do
  DATE=$(date +%s)
  SNMP=$(cat /proc/net/snmp | grep Udp: | grep -v InDatagrams)
  ETHT=$(ethtool -S $dev | grep rx_no_buffer_count)
  ETHPKT=$(ethtool -S $dev | grep rx_packets)

  INDG=$(echo $SNMP | awk '{print $2}')
  INERR=$(echo $SNMP | awk '{print $4}')
  NICERR=$(echo $ETHT | awk '{print $2}')
  NICPKT=$(echo $ETHPKT | awk '{print $2}')

  echo "efu.net.udp_rx $INDG $DATE"
  echo "efu.net.udp_rxerr $INERR $DATE"
  echo "efu.net.nic_rxerr $NICERR $DATE"
  echo "efu.net.nic_rx $NICPKT $DATE"

  echo "efu.net.udp_rx $INDG $DATE" | nc $ip $port
  echo "efu.net.udp_rxperr $INERR $DATE" | nc $ip $port
  echo "efu.net.nic_rxerr $NICERR $DATE" | nc $ip $port
  echo "efu.net.nic_rx $NICPKT $DATE" | nc $ip $port
  sleep 2
done
