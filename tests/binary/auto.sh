#! /bin/sh
# auto.sh:  Tests for "admin"'s detection of binary files.

# Import common functions & definitions.
. ../common/test-common.sh
. ../common/real-thing.sh
. ../common/config-data.sh

# test_bin:
# Usage:   test_bin LABEL <contents>
#
# create a file containing the specified argument and check
# that it is encoded as a binary file.
test_bin() {
    set_and_maybe_print_step_label_with_dots "${1}"
    shift

    rm -f -- infile "${s}"
    echo_nonl "$@" > infile
    if ${vg_admin} -iinfile ${adminflags} $s >/dev/null 2>&1
    then
	if ( ${prs} -d:FL: "${s}" 2>/dev/null; echo foo ) | grep encoded >/dev/null 2>&1
	then
            echo passed
	else
            fail "${label}" "input did not produce an encoded s-file."
	fi
    else
	fail "${label}" "${vg_admin} returned exit status ${?}."
    fi
    rm -f -- infile "${s}"
}

# test_ascii:
#
# As for test_bin, but the resulting SCCS file must NOT be encoded.
#
test_ascii() {
    set_and_maybe_print_step_label_with_dots "${1}"
    shift

    rm -f infile "${s}"
    echo_nonl "$@" > infile
    if ${vg_admin} -iinfile ${adminflags} "${s}" >/dev/null 2>&1
    then
	if ( ${prt} -f $s 2>/dev/null ; echo foo ) | grep encoded >/dev/null 2>&1
	then
            fail "${label}" "input produced an encoded s-file and should not have."
	else
            echo passed
	fi
    else
	fail "${label}" "${vg_admin} returned exit status ${?}."
    fi
    rm -f -- infile "${s}"
}

g=testfile
s="s.${g}"
z="z.${g}"
x="z.${g}"
p="p.${g}"

remove command.log log  base "${g}" "${s}" "${z}" "${x}" "${p}"

adminflags=""

if test -z "${binary_support}"
then
    abandon_test_script "Variable binary_support should have been set to either true or false by config-data.sh."
fi

if $binary_support
then
    test_ascii a1 "foo\n"
    test_ascii a2 "foo\nbar\n"
    test_ascii a3 ""                        # an empty input file should be OK.
    test_bin   b4 "\001\n"                  # line starts with control-a
    test_ascii a5 "x\001\n"                 # line contains ^A but doesn't start with it.

    adminflags=-b
    test_bin   b6 "foo\n"                   # Test manual encoding override.
    test_bin   b7 "x\000y\n"                # ASCII NUL.
    test_bin   b8 "\000"                    # Just the ASCII NUL alone.
    test_bin   b9 "\001"                    # Just the ^A alone.
    adminflags=

    if $TESTING_CSSC
    then
	## Real SCCS fails on these inputs:-
	test_bin   fb10 "foo"               # no newline at end of file.
	test_ascii fa11 "x\000y\n"          # ASCII NUL.
    else
	echo "Some tests skipped (since SCCS fails them but CSSC should pass)"
    fi
else
    echo "No binary file support -- tests skipped"
fi # binary support.

remove command.log "${s}" "${p}" "${g}" "${z}" "${x}" infile
success
