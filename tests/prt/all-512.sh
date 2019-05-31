#! /bin/sh
. ../common/test-common
. ../common/need-prt
export prt

cp testfile2_s s.testfile2 || miscarry 'could not prepare test input s.testfile2'

sh all-variations.txt 2>&1 >got.stdout | 
    grep -v "feature not fully tested: excluded delta"

remove all.expected

/bin/sh ../../testutils/decompress_stdin.sh <all.expected.Z >all.expected \
    || miscarry could not decompress expected output 

if diff all.expected got.stdout >/dev/null 
then
    remove all.expected s.testfile2
    success
else
    echo "output differs --"
    diff -c all.expected got.stdout | head -30
    fail output differs
fi
