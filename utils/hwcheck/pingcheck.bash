#!/bin/bash

mtusize=${1:-9000}
icmpechosize=$((mtusize - 28))

echo mtu size $mtusize
echo ping size $icmpechosize

servers=" 172.24.0.201 172.24.0.202 172.24.0.203 172.24.0.204 172.24.0.205 172.24.0.206"


function errexit() {
    echo "Error: "$1
    exit 1
}

# could add -i 0.5 - if it worked
pingoptions="-M do -c 2 "

echo "Checking server for icmp echo with payload size of $icmpechosize"
failures=0
for server in $servers
do
   echo -n "pinging server $server ... "
   ping $pingoptions -s $icmpechosize $server &>/dev/null 
   if [[ "$?" -eq "0" ]]; then
       echo PASSED
   else
       failures=$((failures + 1)) 
       echo FAILED 
   fi
done

echo failures: $failures

if [[ "$failures" -ne "0" ]]; then
    errexit "unable to ping $failures server[s]  with -c $icmpechosize"
fi
