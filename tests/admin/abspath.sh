#! /bin/sh

# abspath.sh:  Testing for running admin when the s-file 
#              is specified by an absolute path name.

# Import common functions & definitions.
. ../common/test-common

remove s.bar 

docommand P1 "${admin} -n `../../testutils/realpwd`/s.bar" 0 "" IGNORE

remove s.bar 
success

