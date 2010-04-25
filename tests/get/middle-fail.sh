#! /bin/sh
#
# This is a test for SourceForge Bug ID 595594, reported by Joel Young.
# This is where "sccs get SCCS" where there are three fiules (a, b, c) in the
# SCCS difrectory stops processing at b, because a writable version of 
# b exists.  In fact iot should carry on a check out a copy of c.

. ../common/test-common
. ../common/not-root


files="a b c"


cleanup () {
    if [ -d SCCS ] 
    then
	( cd SCCS && for i in $files; do rm -f [spzd].$i; done )
	rm -f $files
	rmdir SCCS
    fi
    rm -f $files
}

cleanup
remove command.log log log.stdout log.stderr 
mkdir SCCS

echo "Creating the input files..."
for i in $files
do
    echo "This is file $i" > $i
    ${admin} -i$i SCCS/s.$i
    rm $i
done


docommand e1 "${vg_get} -e SCCS/s.b" 0 IGNORE IGNORE
docommand e2 "test -w b" 0 "" ""
docommand e3 "${vg_get} SCCS" 1 IGNORE IGNORE

# At this point, a read-only copy of a and c should exist.
# b should still be writable. 

for i in a c 
do
    docommand e4${i}1 "test -f $i" 0 "" ""
    docommand e4${i}2 "test -w $i" 1 "" ""
done

docommand e5 "test -w b" 0 "" ""

cleanup
success


    
