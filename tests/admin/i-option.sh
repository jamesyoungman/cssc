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


# If the file specified by -i does not exist, make sure that not
# only is there a fatal exit, but neither the s-file or the x-file is 
# left behind.
remove foo s.foo x.foo 
docommand I5 "${admin} -ifoo s.foo" 1 "" IGNORE

echo_nonl "I6..."
if test -f s.foo; then
    fail I6: The file s.foo should not have been created.
fi
echo 'passed'

echo_nonl "I7..."
if test -f x.foo; then
    fail I7: The temporary file x.foo should have been deleted.
fi
remove foo s.foo x.foo 
echo 'passed'



remove s.bar foo bar command.log 
success

