#! /bin/sh

# default.sh:  Tests for prt no involving several deltas.

# Import common functions & definitions.
. ../common/test-common
. ../common/need-prt

s=s.testfile

remove $s
../../testutils/uu_decode --decode < testfile.uue || miscarry could not uudecode testfile.uue.

# XXX: the IGNORE in the followig lines is because of the warning message we
#      get about the excluded deltas feature not being fully tested.

do_output d1 "${prt} -u $s" 0 expected/nodel.-u IGNORE
do_output d2 "${prt} -f $s" 0 expected/nodel.-f IGNORE
do_output d3 "${prt} -t $s" 0 expected/nodel.-t IGNORE
do_output d4 "${prt} -b $s" 0 expected/nodel.-b IGNORE

do_output d5 "${prt} -u -f $s" 0 expected/nodel.-u-f IGNORE
do_output d6 "${prt} -t -b $s" 0 expected/nodel.-t-b IGNORE
do_output d7 "${prt} -u -f -t -b $s" 0 expected/nodel.-u-f-t-b IGNORE

remove $s
success
