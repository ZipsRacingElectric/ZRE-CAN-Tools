#!/bin/bash

if [[ $(/usr/bin/id -u) -ne 0 ]]; then
	echo "Not running as root"
	exit
fi

ip link set down can0
ip link set up can0 type can bitrate 500000