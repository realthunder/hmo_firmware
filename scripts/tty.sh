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

tty_setup() {
  stty -F $TTYDEV $TTYBAUD raw igncr cs8 min 0 "$@"
}

tty_send() {
  echo "$@" > $TTYDEV
}

tty_run()  {
  tty_setup time $1
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

