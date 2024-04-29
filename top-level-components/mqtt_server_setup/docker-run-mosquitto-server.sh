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

# Read which certificate are to be used.
source "${PRIVATE_DIR}/active_certificates.vars"
# e.g.:
# # Created: 2024-04-28_21-59
# HOSTNAME=gonzo
# EXPIRE_DATE=2025-04-28
# ACTIVE_CERTIFICATES_DIR=gonzo_server_certs_2025-04-28

CERT_DIR="${PRIVATE_DIR}/${ACTIVE_CERTIFICATES_DIR}"
LOG_DIR="${PRIVATE_DIR}/${HOSTNAME}_mosquitto_log"
echo "CERT_DIR=$CERT_DIR"
echo "LOG_DIR=$LOG_DIR"

mkdir --parents "${LOG_DIR}"
set -x

#----------
# Secure
#----------
echo running SECURE mosquitto.
docker run --name mosquitto --rm --detach --network="host"  \
    --volume ${SCRIPT_DIR}/mosquitto_2-0_secure.conf:/mosquitto/config/mosquitto.conf  \
    --volume ${CERT_DIR}:/mosquitto/data  \
    --volume ${LOG_DIR}:/mosquitto/log  \
    eclipse-mosquitto:2-openssl
docker container ls

# To Terminate:
# docker container stop mosquitto
