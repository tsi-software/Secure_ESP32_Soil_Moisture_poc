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
ls -1 --file-type /dev | egrep -i "(acm)|(usb)"
TTY_DEVICE=ttyACM0
#TTY_DEVICE=ttyUSB0

SRC_DIR=${SCRIPT_DIR}/top-level-components/secure_esp32_client
ENV_FILE=${SCRIPT_DIR}/esp-idf-docker_dev.env
#ENV_FILE=${SCRIPT_DIR}/esp-idf-docker_prod.env

IDF_DOCKER_TAG=release-v5.2
#IDF_DOCKER_TAG=release-v5.1
#IDF_DOCKER_TAG=latest

set -x
docker run --rm --interactive --tty \
  --group-add dialout --device=/dev/${TTY_DEVICE} --env ESPTOOL_PORT=/dev/${TTY_DEVICE} \
  --env-file ${ENV_FILE} \
  --volume ${SRC_DIR}:/project/src \
  --volume ${SCRIPT_DIR}/private:/project/private \
  --volume ${SCRIPT_DIR}/tools:/project/tools \
  --workdir /project/src \
  espressif/idf:${IDF_DOCKER_TAG}
