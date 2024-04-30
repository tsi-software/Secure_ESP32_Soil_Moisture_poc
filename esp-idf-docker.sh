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

PRIVATE_DIR=$(realpath "${SCRIPT_DIR}/private")
echo "PRIVATE_DIR=$PRIVATE_DIR"

# Read which certificate are to be used.
source "${PRIVATE_DIR}/active_certificates.vars"
# e.g.:
# # Created: 2024-04-28_21-59
# HOSTNAME=gonzo
# EXPIRE_DATE=2025-04-28
# ACTIVE_CERTIFICATES_DIR=gonzo_server_certs_2025-04-28

CERT_DIR="${PRIVATE_DIR}/${ACTIVE_CERTIFICATES_DIR}"
echo "CERT_DIR=$CERT_DIR"

#TODO: auto-recognize the USB device.
ls -1 --file-type /dev | egrep -i "(acm)|(ptmx)|(usb)"

SRC_DIR=${SCRIPT_DIR}/top-level-components/secure_esp32_client
ENV_FILE=${SCRIPT_DIR}/esp-idf-docker_dev.env
#ENV_FILE=${SCRIPT_DIR}/esp-idf-docker_prod.env

source ${ENV_FILE}
# e.g.:
# ESPTOOL_AFTER=no_reset
# ESPTOOL_CHIP=esp32s3
# ESPTOOL_PORT=/dev/ttyACM0

set -x

docker build --rm --pull \
  --file esp-idf.dockerfile \
  --tag soil-moisture-esp-idf:latest \
  "${SCRIPT_DIR}"

docker run --rm --interactive --tty \
  --group-add dialout --device=${ESPTOOL_PORT} \
  --env-file ${ENV_FILE} \
  --volume ${SRC_DIR}:/project/src \
  --volume ${SCRIPT_DIR}/private:/project/private \
  --volume ${SCRIPT_DIR}/tools:/project/tools \
  --workdir /project/src \
  soil-moisture-esp-idf:latest
