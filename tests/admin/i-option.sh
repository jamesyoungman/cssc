#! /bin/sh

# i-option.sh:  Testing for correct operation of admin -i.

# Import common functions & definitions.
. ../common/test-common

remove s.bar foo bar

remove foo
echo '%M%' > foo
test `cat foo` = '%M%' || miscarry cannot create file foo.

docommand I1 "${admin} -ifoo s.bar" 0 "" IGNORE
docommand I2 "${get} -r1.1 -p s.bar"      0 "bar\n" IGNORE
remove foo s.bar

# -i on its own means read from stdin.
echo baz | \
docommand I3 "${admin} -i s.bar" 0 "" IGNORE
docommand I4 "${get} -r1.1 -p s.bar"      0 "baz\n" IGNORE

remove s.bar foo bar command.log 

success

