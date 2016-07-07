[ "$TTYDEV" = "" ] && TTYDEV=/dev/ttyUSB0
[ "$TTYLOCK" = "" ] && TTYLOCK=/tmp/`basename $TTYDEV`.lock
[ "$TTYBAUD" = "" ] && TTYBAUD=9600

if [ -z "$TTYINCLUDED" ]; then
  exec 100> $TTYLOCK
  TTYINCLUDED=1
fi

tty_lock() {
  flock 100
}

tty_unlock() {
  flock -u 100
}

tty_send() {
  stty -F $TTYDEV $TTYBAUD raw igncr cs8 min 0
  echo "$@" > $TTYDEV
}

tty_run()  {
  stty -F $TTYDEV $TTYBAUD raw igncr cs8 time $1 min 0
  shift
  (
    echo "$@" > $TTYDEV

    # the first line is the echoed command, so discard it
    read line || return 1

    # echo back the read output until we get the prompt character
    while read line; do
      [ "$line" = ">" ] && return 0
      echo $line
    done
    return 1
  ) < $TTYDEV
}

