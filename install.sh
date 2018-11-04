#!/bin/bash
if (( $# == 2 ));then
    (( num=$1 ))
    (( hashsize=$2 ))
elif (( $# == 1 ));then
    (( num=$1 ))
    (( hashsize=num/1000 ))
    if (( $hashsize == 0 ));then
        hashsize=1
    fi
elif (( $# == 0));then
    num=6000
    hashsize=3
else 
    printf "usage:./install [filenum] [dirnum]\n"
fi

make
./CreateJson

./create_file.sh -f num hashsize


