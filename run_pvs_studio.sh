#!/bin/bash
set -euo pipefail               # Use Bash strict mode.

SRC_DIR=${1}
LOG_FILE="PVS-Studio.log"
ROOT_DIR=$PWD

printf "ROOT_DIR: $ROOT_DIR\n";

# Add comments at the top level
how-to-use-pvs-studio-free -c 3 $SRC_DIR

# Run PVS-Studio analyzer
pushd $SRC_DIR
cmake ./ > /dev/null
pvs-studio-analyzer trace -- make -j5 > /dev/null
pvs-studio-analyzer analyze -j2 -l PVS-Studio.lic -o $LOG_FILE > /dev/null

# Convert the log into a readable format
plog-converter -a GA:1,2 -t tasklist $LOG_FILE
popd
