#!/bin/bash

mtusize=${1:-9000}

echo mtu size $mtusize

function errexit() {
    echo "Error: "$1
    exit 1
}


echo "checking MTU size on ethernet interfaces"

numenos=$(ifconfig | grep eno | wc -l)
echo number of enoX interfaces $numenos
mtumatch=$(ifconfig | grep eno | grep "mtu $mtusize" | wc -l)
echo number of enoX with correct MTU: $mtumatch

if [[ $numenos -ne $mtumatch ]]; then
   errexit "correct MTU on $mtumatch out of $numenos enoX interfaces"
fi

exit 0

