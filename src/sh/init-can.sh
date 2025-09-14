#!/bin/sh
# TODO(Barach): This likely isn't safe with spaces in path

# Helper for printing to stderr (stdout is used for return value).
echoerr() { printf "%s\n" "$*" >&2; }

# Check args
if [ "$1" = "" ]; then
	echoerr "Invalid arguments. Usage:"
	echoerr "  init-can <Baud> <Device>"
	exit -1
fi

# No device specified, attempt SLCAN enumeration
if [ "$2" = "" ]; then

	# Enumerate all potential serial devices
	DEVICES=$(ls /dev | grep 'ttyACM.\|ttyUSB.\|ttyS.')

	for DEVICE in $DEVICES; do

		# Based on the OS, get the device name
		if [ "$OS" = "Windows_NT" ]; then
			# Windows, convert ttyS* to COM*
			DEVICE=$(echo $DEVICE | tr -d -c 0-9)
			DEVICE=COM$(($DEVICE + 1))
		else
			# POSIX, convert ttyS* to /dev/ttyS*
			DEVICE=/dev/$DEVICE
		fi

		# Query the CAN device, checking the return code. If successful, print
		# the name and exit.
		# The '2>/dev/null' suppresses standard error output
		can-dev-cli -q $DEVICE@$1 2>/dev/null && echo $DEVICE@$1 && exit 0
	done

	# No device found, exit
	echoerr "No CAN device was detected."
	exit -1

# SLCAN devices
elif [ "$2" = /dev/tty* ]; then

	# Postfix baudrate to device name
	echo $2@$1
	exit 0

# SocketCAN devices
elif [ "$2" == can* ] || [ "$2" == vcan* ]; then

	# Check user permissions, root is required
	if [ $(/usr/bin/id -u) -ne 0 ]; then
		# Standard permissions, use sudo
		IP_CMD="sudo ip"
	else
		# Root permissions
		IP_CMD="ip"
	fi

	# Determine device arguments
	if [ "$2" == can* ]; then
		IP_ARGS="type can bitrate $1"
	else
		IP_ARGS=""
	fi

	# Bring down the device if it is currently online
	$IP_CMD link set down $2 || exit $?

	# Bring up the device using the specified baudrate
	$IP_CMD link set up $2 $IP_ARGS || exit $?

	echo $2
	exit 0
fi

echoerr "Unrecognized CAN device '$2'"
exit -1