#!/bin/bash
# esp-idf-docker.sh
#

# -e  Exit immediately if a command exits with a non-zero status.
# -u  Treat unset variables as an error when substituting.
# -v  Print shell input lines as they are read.
# -x  Print commands and their arguments as they are executed.
set -e
set -u

shopt -s -o nounset
declare -rx SCRIPT=${0##*/}
SCRIPT_DIR=$(dirname "${0}")
SCRIPT_DIR=$(realpath "${SCRIPT_DIR}")
NOW=$(date +"%Y-%m-%d_%H-%M")
echo "$NOW"
echo "SCRIPT=$SCRIPT"
echo "SCRIPT_DIR=$SCRIPT_DIR"

#TODO: auto-recognize the USB device.
ls -l /dev | grep -i USB

SRC_DIR=${SCRIPT_DIR}/top-level-components/secure_esp32_client

#cd "${SCRIPT_DIR}/top-level-components/secure_esp32_client"
# docker run --rm --group-add dialout --device=/dev/ttyUSB0 -v $PWD:/src -w /src -it espressif/idf:release-v5.1

set -x
docker run --rm --interactive --tty \
  --group-add dialout --device=/dev/ttyUSB0 \
  --volume ${SRC_DIR}:/project/src \
  --volume ${SCRIPT_DIR}/private:/project/private \
  --volume ${SCRIPT_DIR}/tools:/project/tools \
  --workdir /project/src \
  espressif/idf:release-v5.1
