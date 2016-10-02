#!/bin/bash

get_version() {
    local ver
    local head
    local name=$1

    if head=`git rev-parse --verify HEAD 2>/dev/null`; then
        head=`echo "$head"`
        ver=`printf '%s' "$head" | cut -c1-8`
        if ! git status -uno | grep "nothing to commit" >/dev/null ; then
            ver="$ver"M
        fi
    fi

    cat > $name.tmp <<TMP_VER
#define HMO_REVISION $ver
TMP_VER
    if `diff $name $name.tmp > /dev/null` ; then    
        rm $name.tmp
    else
        mv -f $name.tmp $name
    fi
}

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
    get_version revision.h
    make $args || exit
    if test $flash; then
        . scripts/tty.sh
        tty_send ${flash}reboot
        make $args upload
    fi
fi
