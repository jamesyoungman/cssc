#!/bin/sh

set -e

for i in admin delta get prs unget 
do 
    echo ============== Tests for $i ==================
    ret=`pwd`
    cd $i

    for s in *.sh
    do
	echo ------------ $i/$s ---------------------
	sh $s
	echo ------------ pass: $i/$s ---------------
    done

    cd $ret
    echo ============== PASS: $i ==================
done
exit 0
