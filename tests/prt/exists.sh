#! /bin/sh

# exists.sh:  What if the input file doesn't exist?

# Import common functions & definitions.
. ../common/test-common.sh
. ../common/need-prt.sh

s=s.testfile

remove $s
docommand e1 "${vg_prt} $s" 1 "IGNORE" "IGNORE"
remove $s

success
