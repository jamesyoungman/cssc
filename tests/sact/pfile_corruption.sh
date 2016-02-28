#! /bin/sh

# pfile_corruption.sh: Tests relating to p-file corruption

# Import common functions & definitions.
. ../common/test-common
. ../common/real-thing

# Local functions
# Manually create a valid p-file
makep() {
    printf "$1" >| "${p}"
}

check_pfile() {
    options=""
while case "$1" in
	  -*) options="$options $1"; true;;
	  *) false;;
      esac
do
    shift
done

    label="$1"
    pbody="$2"
    sact_retcode="$3"
    expected_stdout_egrep_pattern="$4"
    expected_stderr_egrep_pattern="$5"
    shift 5
    if ! $TESTING_CSSC; then
	# Other implementations have different error messages.
	expected_stderr_egrep_pattern=IGNORE
    fi
    makep "$pbody"
    docommand --stderr_regex --stdout_regex $options "$label" "${vg_sact} $s" $sact_retcode "$expected_stdout_egrep_pattern" "$expected_stderr_egrep_pattern"
}

g=foo
s=s.$g
p=p.$g

remove $s $p $g

echo >| $g

# Create an s-file with a revision 1.1.
docommand setup1 "${vg_admin} -i${g} ${s}" 0 "" IGNORE
# Remove the writable g-file so that get -e won't barf.
remove $g

# Use get to create a valid p-file
docommand setup2 "${vg_get} -e ${s}" 0 IGNORE IGNORE

# Verify that everything seems to work.  Ignore stdout since it will
# contain the date/time at which we ran command setup2, which we can't
# predict.
docommand S1 "${vg_sact} $s" 0 IGNORE ""

# This is the basic success case (without using check_pfile)
docommand S2 "makep '1.1 1.2 james 16/02/28 10:59:47\n'" 0 IGNORE IGNORE
docommand S3 "${vg_sact} $s" 0 '1.1 1.2 james 16/02/28 10:59:47\n' IGNORE

# At this point we know that sact doesn't barf on a valid p-file and
# we can make a valid p-file and use sact to display it.

# This just verifies that check_pfile actually can succeed (with no
# stderr message)
check_pfile  --nostderr_regex S4 '1.1 1.2 james 16/02/28 10:59:47\n' 0 IGNORE ""

# Verify that it's OK for the date in the pfile to be in the past.
# We later rely on this fact in test S9.
check_pfile --nostdout_regex S4a '1.1 1.2 james 15/02/28 10:59:47\n' 0 '1.1 1.2 james 15/02/28 10:59:47\n' IGNORE

# Verify that we detect a missing terminating newline
check_pfile S5 '1.1 1.2 james 16/02/28 10:59:47' 1 IGNORE IGNORE

# Verify that we detect an invalid initial or next SID
check_pfile S6 'x.y 1.2 james 16/02/28 10:59:47\n' 1 IGNORE "SID"
check_pfile S7 '1.1 x.y james 16/02/28 10:59:47\n' 1 IGNORE "SID"

# The username is basically free-format, just make sure it's not missing.
check_pfile S8 '1.1 1.2 16/02/28 10:59:47\n' 1 IGNORE "Expected 5-7 args"

# Invalid date (there are not that many days in February 2015)
check_pfile S9 '1.1 1.2 james 15/02/29 10:59:47\n' 1 IGNORE "Invalid date"

# Valid time, 24 clock
check_pfile --nostdout_regex --nostderr_regex S10 '1.1 1.2 james 16/02/28 23:59:47\n' 0 \
	    '1.1 1.2 james 16/02/28 23:59:47\n' ""

# Invalid times
check_pfile S11 '1.1 1.2 james 16/02/28 25:59:47\n' 1 IGNORE "Invalid date/time"
check_pfile S12 '1.1 1.2 james 16/02/28 23:69:47\n' 1 IGNORE "Invalid date/time"
check_pfile S13 '1.1 1.2 james 16/02/28 23:49:67\n' 1 IGNORE "Invalid date/time"

# Make sure we allow a leap second.
check_pfile  --nostdout_regex --nostderr_regex S14 '1.1 1.2 james 16/02/28 23:59:60\n' 0 \
	     '1.1 1.2 james 16/02/28 23:59:60\n' ""

# Included and excluded deltas
# This test shows that the included/excluded deltas are not printed by sact.
# This follows the current implementation of CSSC, I did not yet cross check
# by running this test against Unix.
check_pfile  --nostdout_regex --nostderr_regex S15 \
	     '1.3 1.4 james 16/02/28 11:55:42 -i1.1-1.2 -x1.3\n' 0 \
	     '1.3 1.4 james 16/02/28 11:55:42\n' ""
# Just included deltas
check_pfile   --nostdout_regex  --nostderr_regex S16 '1.3 1.4 james 16/02/28 11:55:42 -i1.1-1.2\n' 0 IGNORE ""
# Just excluded deltas
check_pfile  --nostderr_regex S17 '1.3 1.4 james 16/02/28 11:55:42 -x1.1-1.2\n' 0 IGNORE ""
# Included and excluded deltas, but in the other order
check_pfile  --nostderr_regex S18 '1.3 1.4 james 16/02/28 11:55:42 -x1.1-1.2 -i1.3\n' 0 IGNORE ""


exit 0

# Cleanup
remove $s $p $g
