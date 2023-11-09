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

echo "$NOW"

cd "${SCRIPT_DIR}"
set -x

if [ ! -d ".venv" ] ; then
    python3.11 -m venv .venv
fi

#source .venv/bin/activate
#python3 -m pip install --upgrade pip
#python3 -m pip install --requirement requirements.txt
pip install --upgrade pip
