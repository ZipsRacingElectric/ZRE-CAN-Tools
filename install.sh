#!/bin/bash

# Shell script to be executed on user login. This is meant to define environment variables and add the installation to the
# system path.
SCRIPT=/etc/profile.d/zre_cantools.sh

# Delete the script if it already exists
rm -f $SCRIPT

# Start of script
echo "#!/bin/bash" >> $SCRIPT

# Define the ZRE_CANTOOLS_DIR variable
echo "export ZRE_CANTOOLS_DIR=$PWD" >> $SCRIPT

# Append the bin directory to the system path
echo "export PATH=\$PATH:$PWD/bin" >> $SCRIPT
