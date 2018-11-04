#!/bin/bash
#按照参数num创建num个文件 
#由于文件过多 将文件hash到不同的目录中存储

#在cpp文件中的hash逻辑
#int _hash_(const string& filename)
#{
#    int ret = 0;
#    string::const_iterator it = filename.begin();
#    while(it != filename.end())
#    {
#        ret = ret*3 + (*it) - '0';
#        it++;
#    }
#    return ret % HASHSIZE;
#}

filenum=1000
HASHSIZE=1000
if (( $# < 1 || $# > 2));then
    printf " usage :creat_file.sh filenum [dirnum 'fefault=1000']\n"
    exit
fi

if (( $# == 2));then
    let filenum=$2
    let HASHSIZE=$2
fi

#记录hash数量
rm -f hash_config.cfg
touch hash_config.cfg
printf "$HASHSIZE" >> hash_config.cfg

./CreateJson

rm -f ./booklist
touch booklist
num=$1
printf "\n"
printf "=====================================\n"
basepath=$(cd `dirname $0`; pwd)
printf ">get in $basepath\n"
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

while (( i<filenum ))
do
    dirname="dir"${i}
    mkdir ${dirname}
    let i++
done

hash=0
let i=0
while (( i<num ))
do
    hash=0
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
        (( hash = hash*2 + next ))
        name=$name$next
        let j++
    done
#    touch $name
#   将text.json中的内容复制过来 并替换id为当前文件名id
    (( hash = hash%HASHSIZE ))
#    printf "${hash}\n"
    if [ ! -f ./"dir"$hash/$name ] ;then
        printf "${name}\n" >> ../booklist
        cp ${basepath}"/text.json" ./"dir"$hash/$name
        sed -i "s/9787115147318/${name}/g" ./"dir"$hash/$name
        let i++;
        printf ">[${i}/${num}] creat $name \n" 
    fi
done
printf "get out $bookfile\n"
printf ">down\n"
