#!/bin/bash

TEMP_DIR="temp"
RUNNABLE="reminder"
PID_PATH="/var/run/reminder_daemon.pid"

sudo touch $PID_PATH
sudo chmod 0666 $PID_PATH
rm reminder
mkdir $TEMP_DIR
cd $TEMP_DIR
cmake ..
make
cd ..
cp $TEMP_DIR/$RUNNABLE .
rm -r $TEMP_DIR