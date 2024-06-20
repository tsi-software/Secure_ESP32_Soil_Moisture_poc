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

DOCKER_TAG=soilmoisture_listener:v0.0.1
CONTAINER_NAME=soil

cd "${SCRIPT_DIR}"
set -x

docker build --rm --pull --tag ${DOCKER_TAG} .

# docker run --rm --name ${CONTAINER_NAME} --detach \
#   --env "TZ=${TZ}" \
#   --volume "${SCRIPT_DIR}/../../private":/soilmoisture_listener/private \
#   --volume "${SCRIPT_DIR}/../../private/config.ini":/soilmoisture_listener/config.ini \
#   --volume "${SCRIPT_DIR}/../../output_data":/soilmoisture_listener/output_data \
#   ${DOCKER_TAG}

#TODO: this command is temporary and should be removed in favor of the command just above.
docker run --rm --name ${CONTAINER_NAME} --detach \
  --env "TZ=${TZ}" \
  --volume "${SCRIPT_DIR}/private":/soilmoisture_listener/private \
  --volume "${SCRIPT_DIR}/private/config.ini":/soilmoisture_listener/config.ini \
  --volume "${SCRIPT_DIR}/../../output_data":/soilmoisture_listener/output_data \
  ${DOCKER_TAG}

docker container logs ${CONTAINER_NAME} --follow

# To gain shell access:
#docker exec -it soil /bin/bash
#  or
#docker exec -it soil /bin/sh
