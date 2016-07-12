#!/bin/bash

board=
flash=
nosync=
args=
for arg in "$@"; do
    case $arg in
    nosync)
        nosync=1
        ;;
    board=*)
        board=${arg#board=}
        args="$args $arg"
        ;;
    flash=*)
        flash=${arg#flash=}
        ;;
    *)
        args="$args $arg"
        ;;
    esac
done

if test $board && test -z $nosync; then
    base=`basename $PWD`
    path="works/arduino/code"
    rsync='rsync -zavrl --partial --exclude=*.sw* --exclude=*build-* --progress --no-p --chmod=ugo=rwX' 
    for p in ../Arduino-Makefile ../libraries ../$base; do
        $rsync $p precise642:$path || exit
    done
    ssh precise642 "cd $path/$base && ./make.sh nosync $@"
else
    make $args || exit
    if test $flash; then
        . scripts/tty.sh
        tty_send ${flash}reboot
        make $args upload
    fi
fi
