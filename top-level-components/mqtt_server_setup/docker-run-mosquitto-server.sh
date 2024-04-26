#!/bin/bash
# docker-run-mosquitto-server.sh
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

PRIVATE_DIR=$(realpath "${SCRIPT_DIR}/../../private")
echo "PRIVATE_DIR=$PRIVATE_DIR"

cd "${SCRIPT_DIR}"
mkdir --parents mosquitto_log

CERT_DIR="${PRIVATE_DIR}/gonzo_server_certs_2024-08-05"

set -x

#----------
# Secure
#----------
echo running SECURE mosquitto.
docker run --name mosquitto --rm --detach --network="host"  \
    --volume ${SCRIPT_DIR}/mosquitto_2-0_secure.conf:/mosquitto/config/mosquitto.conf  \
    --volume ${CERT_DIR}:/mosquitto/data  \
    --volume ${SCRIPT_DIR}/mosquitto_log:/mosquitto/log    \
    eclipse-mosquitto:2-openssl
docker container ls

# To Terminate:
# docker container stop mosquitto
