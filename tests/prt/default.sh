#! /bin/sh

# default.sh:  Test the default behaviour of prt.

# Import common functions & definitions.
. ../common/test-common

s=s.testfile

remove $s
uudecode < testfile.uue || miscarry could not uudecode testfile.uue.


do_output d1 "${prt} $s" 0 expected/default.1 ""

success
