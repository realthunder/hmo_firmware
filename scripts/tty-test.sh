#!/bin/bash
. tty.sh

# variable to hold tty output
ans=

prompt() {
    echo "press enter to continue..."
    read line
}

run() {
    echo "> $@"
    # calling tty_run with 1 second timeout
    if ! ans=`tty_run 10 "$@"`; then
        echo timeout
    else 
        echo "< $ans"
    fi
    echo
}

echo "Power up the first device now. It shall response with 'nil'"
prompt
run "id"

echo "Setting the device id to ascii 97 = 'a'"
prompt
run "id(97)"

echo "From now on, device will only response to command prefix with 'a'"
prompt
run "aid"

echo "Now, power up the second device. It shall response with \\0"
echo "If no new device is up, it shall timeout"
prompt
run "id"

echo "Setting the second device to 'b'"
prompt
run "id(98)"

echo "Confirm the second device id is set"
prompt
run "bid"

echo "Testing the broadcast feature by prefix command with '0'."
# broadcast command shall be interpreted by all devices online, but no
# device shall respond to avoid bus contention. So we don't use tty_run 
# which will wait for response. Use simple tty_send instead.
prompt
echo "> 0id"
tty_send "0id"
echo

echo "Broadcast id command shall increase the device id by one."
echo "So now, device 'a' become device 'b', and device 'b' become device 'c'"
prompt
run "bid"
run "cid"

echo "And of course, no device 'a' now"
prompt
run "aid"

