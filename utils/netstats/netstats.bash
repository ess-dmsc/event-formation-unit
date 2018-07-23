#!/bin/bash

# on mjc workstation, try 172.17.12.31 2003 enp0s25

ip=$1
port=$2
dev=${3:-enp0s31f6}

function errexit() {
    echo error: $1
    exit 1
}

# Check that ethtool exists
ethtool -h &>/dev/null || errexit "ethtool is not available, please install it"
echo "ethtool available (ok)"

# Check that device exists
ethtool -S $dev &>/dev/null || errexit "device: $dev does'nt exist"
echo "device $dev available (ok)"

# Get the driver specific pattern match for counter
driver=$(ethtool -i $dev | grep driver | awk '{print $2}')

case $driver in
e1000e)
  # Ubuntu + driver e1000e
  echo e100e found - OK
  PATT_NICERR=rx_no_buffer_count
  PATT_NICRX=rx_packets
  ;;
tg3)
  # Ubuntu + tg3 
  echo tg3 found - STATS NOT VERIFIED
  PATT_NICERR=rx_discard
  PATT_NICRX=rx_ucast_packets
  ;;
*)
  errexit "unknown driver for device $dev - no stats available"
  ;;
esac
PATT_UDPRX=InDatagrams

echo "driver for device $dev available (ok) - entering main loop"

while [[ 1 ]]
do
  DATE=$(date +%s)
  SNMP=$(cat /proc/net/snmp | grep Udp: | grep -v ${PATT_UDPRX})
  ETHT=$(ethtool -S $dev | grep ${PATT_NICERR})
  ETHPKT=$(ethtool -S $dev | grep ${PATT_NICRX})

  INDG=$(echo $SNMP | awk '{print $2}')
  INERR=$(echo $SNMP | awk '{print $4}')
  NICERR=$(echo $ETHT | awk '{print $2}')
  NICPKT=$(echo $ETHPKT | awk '{print $2}')

  echo "efu.net.udp_rx $INDG $DATE"
  echo "efu.net.udp_rxerr $INERR $DATE"
  echo "efu.net.nic_rxerr $NICERR $DATE"
  echo "efu.net.nic_rx $NICPKT $DATE"

  echo "efu.net.udp_rx $INDG $DATE" | nc -q 0 $ip $port
  echo "efu.net.udp_rxperr $INERR $DATE" | nc -q 0 $ip $port
  echo "efu.net.nic_rxerr $NICERR $DATE" | nc -q 0 $ip $port
  echo "efu.net.nic_rx $NICPKT $DATE" | nc -q 0 $ip $port
  sleep 2
done
