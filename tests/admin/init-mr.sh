#! /bin/sh

# init-mr.sh:  Testing for MR numbers at initialisation time.

# Import common functions & definitions.
. ../common/test-common

s=s.new.txt

remove foo $s new.txt

# Create an empty SCCS file to work on.
docommand I1 "${admin} -i/dev/null $s" 0 "" IGNORE

# get it, make sure it's revision 1.1 and empty.
docommand I2 "${get} -r1.1 -p $s" 0 "" IGNORE
remove $s

# Recreate it with a keyword.
echo '%M%' >foo
docommand I3 "${admin} -ifoo $s" 0 "" ""

# Make sure that worked, we got the %I%, and we get no error.
docommand I4 "${get} -r1.1 -p $s" 0 "new.txt\n" "1.1\n1 lines\n"
remove $s

# Use the -r option to set the initial SID.
docommand I5 "${admin} -ifoo -r2 $s" 0 "" ""

# Make sure that worked, we got the %I%, and we get no error.
docommand I6 "${get} -p $s" 0 "new.txt\n" "2.1\n1 lines\n"
remove new.txt

# Check that the MR validation flag is OFF.
docommand I6a "${prs} -d:MF: $s" 0 "no\n" ""

# Check that the MR validation program is unset.
docommand I6b "${prs} -d:MP: $s" 0 "none\n" ""

# We should not be able to admin -i if the s-file already exists.
docommand I7 "${admin} -ifoo $s" 1 "" IGNORE
remove $s

##
## OK, we know the -i and -r options work.
## Make sure -r doesn't work without an argument.
docommand I8 "${admin} -ifoo -r 2 $s" 1 "" "IGNORE"
test  -f $s && fail I8 stage I8 should not have created $s.

## Create and specify MR numbers...

# No MR
docommand I9 "${admin} -fv -m -r2 -ifoo $s" 0 "" ""
# Check for absence of MRs
docommand I10 "${prs} $s | sed -ne '/^MRs:$/,/^COMMENTS:$/ p'" \
    0  "MRs:\nCOMMENTS:\n" ""

# One MR -- v flag unset, should fail.
remove $s
docommand I13 "${admin} -m13 -ifoo $s" 1 "" IGNORE
test  -f $s && fail I13b stage I13 should not have created $s.

# Set MR flag -- should work.
remove $s
docommand I14 "${admin} -fv -m13 -ifoo $s" 0 "" IGNORE
# Check for correct MRs
docommand I15 "${prs} $s | sed -ne '/^MRs:$/,/^COMMENTS:$/ p'" \
    0  "MRs:\n13\nCOMMENTS:\n" ""

# Check that the MR validation flag is ON.
docommand I16 "${prs} -d:MF: $s" 0 "yes\n" ""

# Install MR validating program (setting & getting the 
# name of the MR validator)
docommand I17 "${admin} -fvtrue $s" 0 "" IGNORE

# Check that the MR validation program is set correctly.
docommand I18 "${prs} -d:MP: $s" 0 "true\n" ""

## The actual use of the MR validator is tested in the tests for "delta".

# Make sure validation checks can succeed, ever.
remove $s
docommand I19 "${admin} -fvtrue -m19 -ifoo $s" 0 "" ""

# Check compatible behaviour with regard to MR validation 
# failure at initialisation.
remove $s
docommand I20 "${admin} -fv/bin/false -m20 -ifoo $s" 1 "" IGNORE
test  -f $s && fail I21 stage I20 should not have created $s.

rm -rf test 
remove foo $s new.txt command.log

success

