#!/bin/bash
# create_certificates.sh
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

SOURCE_VARS_FILE="${PRIVATE_DIR}/create_certificates.vars"
source "${SOURCE_VARS_FILE}"
# e.g.:
# DOMAIN=hostname.com
# HOSTNAME=hostname
# EMAIL=email@hostname.com
# REGION=/C=CA/ST=BC/L=Vancouver
# DAYS_VALID=365

if [ -v DAYS_VALID ]; then
    #date --date='+365 days' +%Y-%m-%d
    DAYS_VALID_OPTION="-days ${DAYS_VALID}"
    EXPIRE_DATE=$(date --date="+${DAYS_VALID} days" +"%Y-%m-%d")
    echo "DAYS_VALID=$DAYS_VALID"
    echo "DAYS_VALID_OPTION=$DAYS_VALID_OPTION"
    echo "EXPIRE_DATE=${EXPIRE_DATE}"
else
    DAYS_VALID_OPTION=""
    EXPIRE_DATE="NO-EXPIRY"
    echo "EXPIRE_DATE=${EXPIRE_DATE}"
fi

ACTIVE_CERTIFICATES_VARS="${PRIVATE_DIR}/active_certificates.vars"
ACTIVE_CERTIFICATES_DIR=${HOSTNAME}_server_certs_${EXPIRE_DATE}
TARGET_DIR="${PRIVATE_DIR}/${ACTIVE_CERTIFICATES_DIR}"
echo "TARGET_DIR=$TARGET_DIR"
# Must FAIL HARD if the target dir already exists!
mkdir "${TARGET_DIR}"
chmod go-rwx "${TARGET_DIR}"

# Create and populate the file named "active_certificates.vars" in the private directory.
# This file is used later, and elsewhere, to reference which certificate are to be used.
# Do this AFTER the "mkdir ..." code above just in case the mkdir fails.
echo "# Created: $NOW" > "${ACTIVE_CERTIFICATES_VARS}"
echo "HOSTNAME=${HOSTNAME}" >> "${ACTIVE_CERTIFICATES_VARS}"
echo "EXPIRE_DATE=${EXPIRE_DATE}" >> "${ACTIVE_CERTIFICATES_VARS}"
echo "ACTIVE_CERTIFICATES_DIR=${ACTIVE_CERTIFICATES_DIR}" >> "${ACTIVE_CERTIFICATES_VARS}"

SECURE_LOGFILE="${TARGET_DIR}/readme-secure.txt"
# Redirect stdout ( > ) into a named pipe ( >() ) running "tee"
# see: https://stackoverflow.com/questions/3173131/redirect-copy-of-stdout-to-log-file-from-within-bash-script-itself
exec > >(tee -i "${SECURE_LOGFILE}")
exec 2>&1

PASS_OUT_OPTION="-passout file:${PRIVATE_DIR}/certificate-pass-phrase.txt"
PASS_IN_OPTION="-passin file:${PRIVATE_DIR}/certificate-pass-phrase.txt"
PASSWORD_OPTION="-password file:${PRIVATE_DIR}/certificate-pass-phrase.txt"
echo "PASS_OUT_OPTION=$PASS_OUT_OPTION"
echo "PASS_IN_OPTION=$PASS_IN_OPTION"
echo "PASSWORD_OPTION=$PASSWORD_OPTION"

echo "$NOW"
echo "Expires: $EXPIRE_DATE"
cat "${SOURCE_VARS_FILE}"

cd "${TARGET_DIR}"
cp "${PRIVATE_DIR}/certificate-pass-phrase.txt" "${TARGET_DIR}/"
cp "${SOURCE_VARS_FILE}" certificate.vars
echo "EXPIRE_DATE=${EXPIRE_DATE}" >> certificate.vars

set -x
#-------------------------------------------------------------------------------
# Create an X509 CA key and certificate for self-signing
openssl req -new -x509 -extensions v3_ca -keyout mosq_ca.key -out mosq_ca.crt \
       -subj "${REGION}/O=ca.${DOMAIN}/OU=ca/CN=${HOSTNAME}/emailAddress=${EMAIL}" \
       ${DAYS_VALID_OPTION} ${PASS_OUT_OPTION}
openssl x509 -in mosq_ca.crt -noout -text


#-------------------------------------------------------------------------------
# Generate the MQTT Server private key
openssl genrsa -out mosq_server.key 2048

# Generate the MQTT Server self-signed certificate
openssl req -new -key mosq_server.key -out mosq_server.csr \
       -subj "${REGION}/O=server.${DOMAIN}/OU=server/CN=${HOSTNAME}/emailAddress=${EMAIL}"

# Generate the CA signed certificate to use in the MQTT Mosquitto Server
openssl x509 -req -in mosq_server.csr -CA mosq_ca.crt -CAkey mosq_ca.key -CAcreateserial -out mosq_server.crt \
       ${DAYS_VALID_OPTION} ${PASS_IN_OPTION}


#-------------------------------------------------------------------------------
# Increase the security of the files just created
chmod go-rwx *
