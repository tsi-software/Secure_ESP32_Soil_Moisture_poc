#!/bin/bash
# ~/github/tsi-software/Secure_ESP32_Soil_Moisture_poc/DEL-ME/test-bash-loop.sh

#-------------------------------------------------------------------------------
NUMBER_OF_CLIENTS=3

#for ClientIndex in {1..3}
#for ClientIndex in {1..${NUMBER_OF_CLIENTS}}
for (( ClientIndex=1; ClientIndex<=$NUMBER_OF_CLIENTS; ClientIndex++ ))
do
    echo "Client $ClientIndex"
    MESG="message $ClientIndex"
    echo $MESG
done
echo

for ClientIndex in 'a' 'b' 'c' ;
do
    NAME="client_$ClientIndex"
    echo "NAME=$NAME"
done
echo


#-------------------------------------------------------------------------------
TEST_DIR=/home/not_there
if [ -d "$TEST_DIR" ]; then
    echo "'$TEST_DIR' exists."
else
    echo "'$TEST_DIR' does not exist!"
fi

TEST_DIR=/home/not_there
if [ ! -d "$TEST_DIR" ]; then
    echo "'$TEST_DIR' does not exist!"
else
    echo "'$TEST_DIR' exists."
fi


TEST_DIR=/home/wtaylor
if [ -d "$TEST_DIR" ]; then
    echo "'$TEST_DIR' exists."
else
    echo "'$TEST_DIR' does not exist!"
fi

TEST_DIR=/home/wtaylor
if [ ! -d "$TEST_DIR" ]; then
    echo "'$TEST_DIR' does not exist!"
else
    echo "'$TEST_DIR' exists."
fi
echo


#-------------------------------------------------------------------------------
start_date_str="2015-03-05"
end_date_str="2015-03-11"
start_ts=$(date -d "${start_date_str} UTC" '+%s')
end_ts=$(date -d "${end_date_str} UTC" '+%s')
num_of_day=$(( ($end_ts - $start_ts)/(60*60*24) ))
echo $start_ts
echo $end_ts
echo $num_of_day

#-------------------------------------------------
# #FAIL !
# # $1 = From Date string (e.g. "2015-03-05")
# # $2 = To Date string (e.g. "2015-03-11")
# calculate_days() {
#     from_ts=$(date -d "$1 UTC" '+%s')
#     to_ts=$(date -d "$2 UTC" '+%s')
#     echo $(( ($to_ts - $from_ts)/(60*60*24) ))
# }

# from_date="2015-03-05"
# to_date="2015-03-11"
# days="$(calculate_days(${from_date}, ${to_date}))"
# echo "days = $days"
#-------------------------------------------------

