#! /bin/sh

# default.sh:  Tests for prt no involving several deltas.

# Import common functions & definitions.
. test-common

s=s.testfile

remove $s
uudecode < testfile.uue || miscarry could not uudecode testfile.uue.


do_output d1 "${prt} -u $s" 0 expected/nodel.-u ""
do_output d2 "${prt} -f $s" 0 expected/nodel.-f ""
do_output d3 "${prt} -t $s" 0 expected/nodel.-t ""
do_output d4 "${prt} -b $s" 0 expected/nodel.-b ""

do_output d5 "${prt} -u -f $s" 0 expected/nodel.-u-f ""
do_output d6 "${prt} -t -b $s" 0 expected/nodel.-t-b ""
do_output d7 "${prt} -u -f -t -b $s" 0 expected/nodel.-u-f-t-b ""

success
