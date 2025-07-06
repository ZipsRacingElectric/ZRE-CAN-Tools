#!/bin/bash

# SocketCAN devices
if [[ $ZRE_CANTOOLS_DEV == can* ]] || [[ $ZRE_CANTOOLS_DEV == vcan* ]]; then

	if [[ $1 == "" ]]; then
		BAUD=1000000
	else
		BAUD=$1
	fi

	# Bring up can0 at 1Mbit
	if [[ $(/usr/bin/id -u) -ne 0 ]]; then
		# User permissions, use sudo
		sudo ip link set down $ZRE_CANTOOLS_DEV
		sudo ip link set up $ZRE_CANTOOLS_DEV type can bitrate $1
	else
		# Root permissions
		ip link set down $ZRE_CANTOOLS_DEV
		ip link set up $ZRE_CANTOOLS_DEV type can bitrate $1
	fi

# SLCAN devices
elif [[ $ZRE_CANTOOLS_DEV == /dev/tty* ]]; then
	if [[ $1 != "" ]]; then
		# Split baudrate from device name
		arrDev=(${ZRE_CANTOOLS_DEV//,/ })
		export ZRE_CANTOOLS_DEV=$(echo ${arrDev[0]}),$1
	fi
fi