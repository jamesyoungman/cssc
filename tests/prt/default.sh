#! /bin/sh

# default.sh:  Test the default behaviour of prt.

# Import common functions & definitions.
. ../common/test-common
. ../common/need-prt

s=s.testfile

remove $s
cp testfile_s s.testfile || miscarry 'could not stage test input s.testfile'

do_output d1 "${vg_prt} $s" 0 expected/default.1 IGNORE

remove $s
success
