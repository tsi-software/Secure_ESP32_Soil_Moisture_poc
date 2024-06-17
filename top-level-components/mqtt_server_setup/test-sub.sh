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
#mosquitto_sub --url "mqtt://${MQTT_HOST}:1883/test"
# --url mqtt(s)://[username[:password]@]host[:port]/topic

#mosquitto_sub -p 1883 --debug --verbose --topic "#"


#----------
# Secure
#----------
# sudo mosquitto_sub --url "mqtts://${MQTT_HOST}:8883/test" --debug --verbose --insecure \
#  --cafile ${CERT_PATH}/mosq_ca.crt --cert ${CERT_PATH}/mosq_client.crt --key ${CERT_PATH}/mosq_client.key

# mosquitto_sub --url "mqtts://${MQTT_HOST}:8883/#" --verbose \
#  --cafile ${CERT_PATH}/mosq_ca.crt --cert ${CERT_PATH}/mosq_client.crt --key ${CERT_PATH}/mosq_client.key


# mosquitto_sub --verbose \
#   --host ${MQTT_HOST} --port 8883 \
#   --cafile ${CERT_PATH}/mosq_ca.crt --cert ${CERT_PATH}/client_a/mosq_client.crt --key ${CERT_PATH}/client_a/mosq_client.key \
#   --topic '#'

mosquitto_sub --verbose \
  --host ${MQTT_HOST} --port 8883 \
  --cafile ${CERT_PATH}/mosq_ca.crt --cert ${CERT_PATH}/client_a/mosq_client.crt --key ${CERT_PATH}/client_a/mosq_client.key \
  --topic 'soilmoisture/3f36213a-ec4b-43ea-8a85-ac6098fac883/touchpad/1' \
  --topic 'irrigation/#'


# mosquitto_sub --verbose \
#   --host ${MQTT_HOST} --port 8883 \
#   --cafile ${CERT_PATH}/mosq_ca.crt --cert ${CERT_PATH}/mosq_client.crt --key ${CERT_PATH}/mosq_client.key \
#   --topic '/soilmoisture/517b462f-34ba-4c10-a41a-2310a8acd626/capacitive/11' \
#   --topic '/soilmoisture/517b462f-34ba-4c10-a41a-2310a8acd626/capacitive/12' \
#   --topic '/soilmoisture/517b462f-34ba-4c10-a41a-2310a8acd626/capacitive/13' \
#   --topic '/soilmoisture/517b462f-34ba-4c10-a41a-2310a8acd626/capacitive/14' \
#   --topic '+'  --topic '/+'

# mosquitto_sub --verbose \
#   --host ${MQTT_HOST} --port 8883 \
#   --cafile ${CERT_PATH}/mosq_ca.crt --cert ${CERT_PATH}/mosq_client.crt --key ${CERT_PATH}/mosq_client.key \
#   --topic '/soilmoisture/6f1ef23f-b061-4f05-b59d-c68158e7d966/capacitive/14' \
#   --topic '/soilmoisture/6f1ef23f-b061-4f05-b59d-c68158e7d966/capacitive/12' \
#   --topic '/soilmoisture/6f1ef23f-b061-4f05-b59d-c68158e7d966/capacitive/10' \
#   --topic '/soilmoisture/6f1ef23f-b061-4f05-b59d-c68158e7d966/capacitive/8'  \
#   --topic '+'  --topic '/+'


# From Max:
# "mosquitto_sub version 2.0.18 running on libmosquitto 2.0.18.""
# docker exec --interactive --tty mosquitto ash
# mosquitto_sub --verbose --host Max --port 8883 --protocol-version mqttv5 --topic '#' \
#   --cafile data/mosq_ca.crt --cert data/mosq_client.crt --key data/mosq_client.key \
#   -F "%t '%p' [%P]"
