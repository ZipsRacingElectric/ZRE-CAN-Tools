#!/bin/bash

# SocketCAN devices
if [[ $ZRE_CANTOOLS_DEV == can* ]] || [[ $ZRE_CANTOOLS_DEV == vcan* ]]; then

	# Bring up can0 at 1Mbit
	if [[ $(/usr/bin/id -u) -ne 0 ]]; then
		# User permissions, use sudo
		sudo ip link set down can0
		sudo ip link set up can0 type can bitrate 500000
	else
		# Root permissions
		ip link set down can0
		ip link set up can0 type can bitrate 500000
	fi

# SLCAN devices
elif [[ $ZRE_CANTOOLS_DEV == /dev/tty* ]]; then

	# Split baudrate from device name
	arrDev=(${ZRE_CANTOOLS_DEV//,/ })
	export ZRE_CANTOOLS_DEV=$(echo ${arrDev[0]}),500000
fi