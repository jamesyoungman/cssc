#! /bin/sh
#
# This is a test for SourceForge Bug ID 595594, reported by Joel Young.
# This is where "sccs get SCCS" where there are three files (a, b, c) in the
# SCCS difrectory stops processing at b, because a writable version of
# b exists.  In fact it should carry on a check out a copy of c.

. ../common/test-common.sh
. ../common/not-root.sh


# If LANG is defined but the system is misconfigured, we will produce
# the error message "Error setting locale: No such file or directory".
# If that happens, the test suite will fail.  For this reason, we
# unset the LANG environment variable.  Of course, things being
# printed out in the wrong language would also mess up the results of
# the test suite.
# We want to prevent setlocale(LC_ALL, "") failing:
unset LANG

# We assume that all the files we want to work on are in the
# current directory.
unset PROJECTDIR

echo "Using the driver program ${sccs}"

cleanup_abc_files() {
    for base in a b c
    do
	rm -f -- [spzd]."${base}" "${base}"
    done
}


cleanup () {
    if [ -d SCCS ]
    then
	( cd SCCS && cleanup_abc_files )
	rmdir SCCS
    fi
    cleanup_abc_files
}

cleanup
remove command.log log log.stdout log.stderr
mkdir SCCS

echo "Creating the input files..."
for i in a b c
do
    echo "This is file $i" >| "$i"
    ${sccs} enter "$i"
    rm -f ,"$i"
done


docommand e1 "${sccs} edit b" 0 IGNORE IGNORE
docommand e2 "test -w b" 0 "" ""
docommand e3 "${vg_sccs} get SCCS" 1 IGNORE IGNORE

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
