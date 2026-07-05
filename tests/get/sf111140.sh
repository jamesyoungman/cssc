#! /bin/sh
# included.sh:  Extra tests using the test file from bug number 111140.

# Import common functions & definitions.
. ../common/test-common.sh


g=sf111140_testcase.txt
s=s.$g
x=x.$g
z=z.$g
p=p.$g

remove $g $s $x $z $p

cp sf111140_testcase_s s.sf111140_testcase.txt ||
    abandon_test_script 'could not stage test input s.sf111140_testcase.txt'

# If we check out version 1.16 of the provided file (in which
# a trunk delta includes a delta that was on a trunk) we
# should get the same body as recorded in the file sf111140.wtd.
#
test_base="$( basename "$0" )"
test_dir="$( dirname "$0" )"
case "${test_dir}" in
    .)
	test_dir=`pwd`
esac
label_prefix="$( basename "${test_dir}" )/${test_base}"

do_pair() {
    seq="$1"
    sid="$2"

    if awk "\$1 == $seq {print}" <  sf111140_full.txt |
	sed 's/^[0-9]* //'> wanted.tmp
    then
	do_output "${label_prefix}:f${seq}" "${vg_get} -r${sid} -p $s" 0 wanted.tmp IGNORE
    else
	abandon_test_script "awk failed"
    fi
}

do_output s1 "${vg_get} -r1.16 -p $s"      0 sf111140.wtd IGNORE

do_pair 1 1.1
do_pair 3 1.2
do_pair 4 1.3
do_pair 5 1.4
do_pair 6 1.5
do_pair 7 1.6
do_pair 8 1.7
do_pair 9 1.8
do_pair 10 1.9
do_pair 12 1.10
do_pair 13 1.9.1.1
do_pair 14 1.11
do_pair 15 1.12
do_pair 16 1.13
do_pair 17 1.14
do_pair 18 1.14.1.1
do_pair 19 1.14.1.2
do_pair 20 1.14.1.3
do_pair 21 1.14.2.1
do_pair 22 1.15
do_pair 23 1.14.2.2
do_pair 24 1.14.1.4
do_pair 25 1.14.1.5
do_pair 26 1.16
do_pair 27 1.17
do_pair 28 1.18
do_pair 29 1.19
do_pair 30 1.18.1.1
do_pair 31 1.19.1.1
do_pair 32 1.18.2.1
do_pair 33 1.20
do_pair 34 1.21
do_pair 35 1.22
do_pair 36 1.23

remove $g $s $x $z $p wanted.tmp
success
