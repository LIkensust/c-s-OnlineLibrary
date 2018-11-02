#!/bin/bash
if (( $# != 1));then
    printf " usage :creat_file.sh [num]\n"
    exit
fi
num=$1
printf "\n"
printf "=====================================\n"
basepath=$(cd `dirname $0`; pwd)
printf ">get in $basepath\n"
printf ">input the number of files you want to create\n"
mkdir books 

if (($? != 0)); then
    while : 
    do
    printf ">books is already exit,do you want to remove it[y/n]\n" 
    read select
        if [ "X$select" == "Xy" ] || [ "X$select" == "XY" ];then
            rm -fr books
            printf ">remove successs,drop all files in ./books\n"
            break
        elif [ "X$select" == "Xn" ] || [ "X$select" == "XN" ];then
            exit  
        fi
    done
fi

mkdir books
printf ">create dir ./books/\n"
bookfile=${basepath}"/books"
cd $bookfile
printf ">get in $bookfile\n"

i=0
while (( i<num ))
do
    size=$RANDOM
    name=""
    (( size=$size%19+1  ))
    j=0
    while (( j<size ))
    do
        next=$RANDOM
        (( next=$next%10 ))
        if (( j==0 ));then
            if (( next==0 ));then
                next=1
            fi
        fi    
        name=$name$next
        let j++
    done
    touch $name
    printf ">creat $name\n"
    let i++
done
printf "get out $bookfile\n"
printf ">down\n"
