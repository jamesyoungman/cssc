#! /bin/sh
. ../common/test-common
export prt

remove s.testfile2
uudecode < s.testfile2.uue || miscarry could not uudecode testfile2.uue.

sh all-variations.txt >got.stdout

remove all.expected

uncompress <all.expected.Z >all.expected \
    || miscarry could not decompress expected output 

if diff all.expected got.stdout >/dev/null 
then
    remove all.expected s.testfile2
    success
else
    echo "output differs --"
    diff -u all.expected got.stdout | head -30
    fail output differs
fi


