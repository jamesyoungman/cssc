#! /bin/sh

# Test for Savannah bug #33633 (sccs prs -d":GB:" core dump).


# Import common functions & definitions.
. ../common/test-common

p=sv33633.txt
s=s.${p}
cleanup () {
    remove command.log
}
cleanup

docommand prs_gb_core_1 "${prs} -d:GB: ${s}" 0 "hi 1.1\n\n" IGNORE
