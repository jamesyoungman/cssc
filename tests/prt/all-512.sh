#! /bin/sh
. ../common/test-common
export prt

sh all-variations.txt >got.stdout

remove all.expected

uncompress <all.expected.Z >all.expected \
    || miscarry could not decompress expected output 

if diff all.expected got.stdout >/dev/null 
then
    remove all.expected
    success
else
    echo "output differs --"
    diff -u all.expected got.stdout | head -30
    fail output differs
fi


