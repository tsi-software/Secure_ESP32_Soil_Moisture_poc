#!/bin/bash
# create_client_certificates.sh
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


if [ -d "${SCRIPT_DIR}/../private" ]; then
    # We are currently running inside of the esp-idf Docker shell.
    PRIVATE_DIR=$(realpath "${SCRIPT_DIR}/../private")
    CERTS_BASE_DIR=$(realpath "${SCRIPT_DIR}/../certificates")
else
    # We are currently running inside of a normal bash shell.
    PRIVATE_DIR=$(realpath "${SCRIPT_DIR}/../../private")
    CERTS_BASE_DIR=$(realpath "${SCRIPT_DIR}/../../certificates")
fi
echo "PRIVATE_DIR=$PRIVATE_DIR"
echo "CERTS_BASE_DIR=$CERTS_BASE_DIR"


# Read which certificate are to be used.
source "${PRIVATE_DIR}/active_certificates.vars"
# e.g.:
# # Created: 2024-04-28_21-59
# HOSTNAME=gonzo
# EXPIRE_DATE=2025-04-28
# ACTIVE_CERTIFICATES_DIR=gonzo_server_certs_2025-04-28

CERT_DIR="${CERTS_BASE_DIR}/${ACTIVE_CERTIFICATES_DIR}"
echo "CERT_DIR=$CERT_DIR"

source "${CERT_DIR}/certificate.vars"
# e.g.:
# DOMAIN=hostname.com
# HOSTNAME=hostname
# EMAIL=email@hostname.com
# REGION=/C=CA/ST=BC/L=Vancouver
# DAYS_VALID=365
# EXPIRE_DATE=2025-01-02

if [ ${EXPIRE_DATE} == "NO-EXPIRY" ]; then
    DAYS_VALID_OPTION=""
else
    # Calculate the remaining number of days that the certificate is valid
    # based on todays date and the expiry date.
    TODAY_DATE=$(date +"%Y-%m-%d")
    TODAY_TIMESTAMP=$(date -d "${TODAY_DATE} UTC" '+%s')
    EXPIRE_TIMESTAMP=$(date -d "${EXPIRE_DATE} UTC" '+%s')
    NUM_OF_DAYS=$(( ($EXPIRE_TIMESTAMP - $TODAY_TIMESTAMP)/(60*60*24) ))
    DAYS_VALID_OPTION="-days ${NUM_OF_DAYS}"
fi
echo "DAYS_VALID_OPTION=$DAYS_VALID_OPTION"

PASS_OUT_OPTION="-passout file:${CERT_DIR}/certificate-pass-phrase.txt"
PASS_IN_OPTION="-passin file:${CERT_DIR}/certificate-pass-phrase.txt"
PASSWORD_OPTION="-password file:${CERT_DIR}/certificate-pass-phrase.txt"
echo "PASS_OUT_OPTION=$PASS_OUT_OPTION"
echo "PASS_IN_OPTION=$PASS_IN_OPTION"
echo "PASSWORD_OPTION=$PASSWORD_OPTION"


set -x
#--------------------------------
# Create 2 sets of certificates for the client to allow seamless certificate rotation.
for ClientIndex in 'a' 'b' ;
do
    CLIENT_DIR="${CERT_DIR}/client_$ClientIndex"
    echo "CLIENT_DIR=$CLIENT_DIR"
    mkdir --parents "${CLIENT_DIR}"
    chmod go-rwx "${CLIENT_DIR}"  # Increase the security of the directory just created.
    cd "${CLIENT_DIR}"


    #-------------------------------------------------------------------------------
    # Generate the MQTT Client private key
    openssl genrsa -out mosq_client.key 2048

    # Generate the MQTT Client self-signed certificate
    openssl req -new -key mosq_client.key -out mosq_client.csr \
           -subj "${REGION}/O=client.${DOMAIN}/OU=client/CN=${HOSTNAME}/emailAddress=${EMAIL}"

    # Generate the CA signed certificate to use in the MQTT Client
    openssl x509 -req -in mosq_client.csr -CA ../mosq_ca.crt -CAkey ../mosq_ca.key \
           -CAcreateserial -out mosq_client.crt \
           ${DAYS_VALID_OPTION} ${PASS_IN_OPTION}

    #-------------------------------------------------------------------------------
    # Generate the MQTT Android private key
    openssl genrsa -out mosq_android.key 2048

    # Generate a certificate signing request to send to the CA.
    openssl req -new -key mosq_android.key -out mosq_android.csr \
           -subj "${REGION}/O=android.${DOMAIN}/OU=android/CN=${HOSTNAME}/emailAddress=${EMAIL}"

    # Sign the CSR with your CA key, or send it to the CA:
    openssl x509 -req -in mosq_android.csr -CA ../mosq_ca.crt -CAkey ../mosq_ca.key \
           -CAcreateserial -out mosq_android.crt \
           ${DAYS_VALID_OPTION} ${PASS_IN_OPTION}

    # For Android, create PKCS#12 file (sometimes referred to as PFX files).
    openssl pkcs12 -export -inkey mosq_android.key -in mosq_android.crt -out mosq_android.p12 -name mosq_android \
           ${PASSWORD_OPTION}

    #-------------------------------------------------------------------------------
    # Increase the security of the files just created
    chmod go-rwx *
done
#--------------------------------

echo "Done."
