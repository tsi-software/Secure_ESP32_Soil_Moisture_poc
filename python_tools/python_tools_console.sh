#!/bin/bash
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
TZ=$(cat /etc/timezone)
echo "$NOW"
echo "TZ=$TZ"
echo "SCRIPT=$SCRIPT"

DOCKER_TAG=python_tools:v0.0.1
CONTAINER_NAME=soil

echo "$NOW"

cd "${SCRIPT_DIR}"
set -x

docker build --rm --pull --tag ${DOCKER_TAG} .

docker run --rm --interactive --tty --name ${CONTAINER_NAME} \
  --env "TZ=${TZ}" \
  --volume "${SCRIPT_DIR}/../certificates":/python_tools/certificates \
  --volume "${SCRIPT_DIR}/../private":/python_tools/private \
  --volume "${SCRIPT_DIR}/../private/active_certificates.vars":/python_tools/active_certificates.vars \
  ${DOCKER_TAG}
