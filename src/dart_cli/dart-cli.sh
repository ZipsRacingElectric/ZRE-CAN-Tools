#! /bin/bash

export SSH_REMOTE="zre@192.168.0.1"
export SSH_OPTIONS="-i $ZRE_CANTOOLS_DIR/bin/keys/id_rsa -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o LogLevel=ERROR"

printf "\nTesting Connection... (Ctrl+C to Cancel)\n"
ssh $SSH_OPTIONS $SSH_REMOTE "printf \"Connected.\n\n\""

while [ "$OPTION" != "q" ]; do

	printf "Enter an option:\n"
	printf " l - List all remote log files\n"
	printf " c - Copy all logs locally\n"
	printf " x - Delete all remote log files\n"
	printf " s - Open an SSH connection to the device\n"
	printf " j - Print the device's system journal\n"
	printf " t - Test connection to the device\n"
	printf " q - Quit\n"
	read OPTION

	if [ "$OPTION" == "q" ]; then

		printf ""

	elif [ "$OPTION" == "l" ]; then

		printf "\nRemote Logs:\n\n"
		ssh $SSH_OPTIONS $SSH_REMOTE "ls /home/zre/mdf/"
		printf "\n"
		ssh $SSH_OPTIONS $SSH_REMOTE "df /"
		printf "\n"

	elif [ "$OPTION" == "c" ]; then

		export DEST_DIR=$ZRE_CANTOOLS_LOGGING_DIR/dart_$(date +%Y.%m.%d)
		printf "%s" "\nCopying Logs to '$DEST_DIR'...\n\n"
		mkdir -p "$DEST_DIR"
		scp $SSH_OPTIONS -r $SSH_REMOTE:/home/zre/mdf/ "$DEST_DIR"
		printf "\nDone.\n\n"

	elif [ "$OPTION" == "x" ]; then

		read -r -p "Are you sure? [y/n] " CONFIRMATION
		if [ "$CONFIRMATION" == "y" ]; then

			printf "\nDeleting Logs...\n\n"
			ssh $SSH_OPTIONS $SSH_REMOTE "rm -r /home/zre/mdf/*"
			printf "\nDone.\n\n"

		fi

	elif [ "$OPTION" == "s" ]; then

		printf "\nOpening SSH Connection... (Type \"exit\" to Close)\n"
		exec ssh $SSH_OPTIONS $SSH_REMOTE

	elif [ "$OPTION" == "j" ]; then

		printf "\nFetching System Journal...\n"
		ssh $SSH_OPTIONS $SSH_REMOTE "journalctl -u init_system_jk"
		printf "\n\n"

	elif [ "$OPTION" == "t" ]; then

		printf "\nTesting Connection... (Ctrl+C to Cancel)\n"
		ssh $SSH_OPTIONS $SSH_REMOTE "printf \"Connected.\n\n\""

	else

		printf "Invalid option.\n"

	fi

done