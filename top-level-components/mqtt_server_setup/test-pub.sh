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

#MQTT_HOST=raspberrypi
#CERT_PATH=./${MQTT_HOST}_server_certs_2024-10-07
source "test-pub-sub.vars"

echo "$NOW"
echo "MQTT_HOST=$MQTT_HOST"
echo "CERT_PATH=$CERT_PATH"

cd "${SCRIPT_DIR}"
set -x

#----------
# Insecure
#----------
#mosquitto_pub --url "mqtt://gonzo:1883/test" -m "Hello MQTT"
# --url mqtt(s)://[username[:password]@]host[:port]/topic

#mosquitto_pub -p 1883 -d -t test -m "Hello MQTT"


#----------
# Secure
#----------
# mosquitto_pub --url "mqtts://${MQTT_HOST}:8883/test" -m "Hello MQTT" --debug --insecure \
#  --cafile ${CERT_PATH}/mosq_ca.crt --cert ${CERT_PATH}/mosq_client.crt --key ${CERT_PATH}/mosq_client.key

# mosquitto_pub --debug \
#  --cafile ${CERT_PATH}/mosq_ca.crt --cert ${CERT_PATH}/mosq_client.crt --key ${CERT_PATH}/mosq_client.key \
#  --url "mqtts://${MQTT_HOST}:8883/soilmoisture/6f1ef23f-b061-4f05-b59d-c68158e7d966/touchpad/config/x" -m "123"

# mosquitto_pub --debug --host ${MQTT_HOST} --port 8883 \
#  --cafile ${CERT_PATH}/mosq_ca.crt --cert ${CERT_PATH}/mosq_client.crt --key ${CERT_PATH}/mosq_client.key \
#  -t "soilmoisture/6f1ef23f-b061-4f05-b59d-c68158e7d966/touchpad/2/config/x" -m "123"

DEVICE_ID=6f1ef23f-b061-4f05-b59d-c68158e7d966
CONFIG_MSG="x=123
y=456"
mosquitto_pub --debug --host ${MQTT_HOST} --port 8883 \
 --cafile ${CERT_PATH}/mosq_ca.crt --cert ${CERT_PATH}/mosq_client.crt --key ${CERT_PATH}/mosq_client.key \
 -t "soilmoisture/${DEVICE_ID}/touchpad/config" -m "${CONFIG_MSG}"
# --property PUBLISH user-property SIGNED 1234

