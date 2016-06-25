#!/bin/bash

board=
for arg in "$@"; do
    case $arg in
    board=*)
        board=${arg#board=}
        ;;
    esac
done

if test $board; then
    base=`basename $PWD`
    path="works/arduino/code"
    rsync='rsync -zavrl --partial --exclude=*.sw* --exclude=*build-* --progress --no-p --chmod=ugo=rwX' 
    for p in ../Arduino-Makefile ../libraries ../$base; do
        $rsync $p precise642:$path || exit
    done
    ssh precise642 "cd $path/$base && make $@"
else
    make "$@"
fi
