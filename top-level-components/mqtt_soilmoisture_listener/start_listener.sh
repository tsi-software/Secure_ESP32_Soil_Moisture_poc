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
BASE_DIR=$(realpath "${SCRIPT_DIR}/../..")
NOW=$(date +"%Y-%m-%d_%H-%M")
TZ=$(cat /etc/timezone)
echo "$NOW"
echo "TZ=$TZ"
echo "SCRIPT=$SCRIPT"
echo "BASE_DIR=$BASE_DIR"

DOCKER_TAG=soilmoisture_listener:v0.0.1
CONTAINER_NAME=soil
CONTAINER_DIR=/soilmoisture_listener
echo "CONTAINER_NAME=$CONTAINER_NAME"
echo "CONTAINER_DIR=$CONTAINER_DIR"

cd "${SCRIPT_DIR}"
set -x


docker build --pull --tag ${DOCKER_TAG} .
# --rm  Remove intermediate containers after a successful build


# DELETE THIS.
# docker run --rm --name ${CONTAINER_NAME} --detach \
#   --env "TZ=${TZ}" \
#   --volume "${SCRIPT_DIR}/../../private":/soilmoisture_listener/private \
#   --volume "${SCRIPT_DIR}/../../private/config.ini":/soilmoisture_listener/config.ini \
#   --volume "${SCRIPT_DIR}/../../output_data":/soilmoisture_listener/output_data \
#   ${DOCKER_TAG}


docker run --rm --name ${CONTAINER_NAME} --detach \
  --env "TZ=${TZ}" \
  --volume "${BASE_DIR}/output_data":${CONTAINER_DIR}/output_data \
  --volume "${BASE_DIR}/python_tools":${CONTAINER_DIR}/python_tools \
  --volume "${BASE_DIR}/certificates":${CONTAINER_DIR}/certificates \
  --volume "${BASE_DIR}/private":${CONTAINER_DIR}/private \
  --volume "${BASE_DIR}/private/active_certificates.vars":${CONTAINER_DIR}/active_certificates.vars \
  ${DOCKER_TAG}


#TODO: this command is temporary and should be removed in favor of the command just above.
# docker run --rm --name ${CONTAINER_NAME} --detach \
#   --env "TZ=${TZ}" \
#   --volume "${SCRIPT_DIR}/private":/soilmoisture_listener/private \
#   --volume "${SCRIPT_DIR}/private/config.ini":/soilmoisture_listener/config.ini \
#   --volume "${SCRIPT_DIR}/../../output_data":/soilmoisture_listener/output_data \
#   --volume "${SCRIPT_DIR}/../../python_tools":/soilmoisture_listener/python_tools \
#   ${DOCKER_TAG}

docker container logs ${CONTAINER_NAME} --follow

# To gain shell access:
#docker exec -it soil /bin/bash
#  or
#docker exec -it soil /bin/sh
