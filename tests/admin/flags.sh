#! /bin/sh

# flags.sh:  Testing for setting /unsetting flags.

# Import common functions & definitions.
. test-common

g=new.txt
s=s.$g
p=p.$g
remove foo $s $g $p

###
### Tests for the 'v' flag; see also init-mrs.sh.
###

# Create SCCS file with a substituted keyword.
echo '%M%' >foo
docommand v1 "${admin} -ifoo $s" 0 "" ""

# Check that the MR validation flag is OFF.
docommand v2 "${prs} -d:MF: $s" 0 "no\n" ""

# Check that the MR validation program is unset.
docommand v3 "${prs} -d:MP: $s" 0 "none\n" ""

## Create and specify MR numbers...

# Create with no MR
remove $s
docommand v4 "${admin} -fv -m -r2 -ifoo $s" 0 "" ""
remove $s

# Set MR flag -- should work.
remove $s
docommand v5 "${admin} -fv -mI13 -ifoo $s" 0 "" IGNORE

# Install MR validating program (setting & getting the 
# name of the MR validator)
docommand v6 "${admin} -fv/bin/true $s" 0 "" IGNORE

# Make sure validation checks can succeed, ever.
remove $s
docommand v7 "${admin} -fv/bin/true -mI19 -ifoo $s" 0 "" ""

remove $s $g



###
### Tests for the 'b' flag
###
docommand b1 "${admin} -ifoo $s" 0 "" ""

# By default branch creation should fail, and we just get a delta
# further down the trunk -- the invocation does not  fail,
# we just don't get a branch.
docommand b2 "${get} -e -b -r1.1 $s" 0 "1.1\nnew delta 1.2\n1 lines\n" ""

# echo testing -- early exit.
# exit 1

docommand b3 "${unget} $s" 0 "1.2\n" ""

# Turn on the enable-branches flag.
docommand b4 "${admin} -fb $s" 0 "" ""

# Create a branch.
docommand b5 "${get} -e -b -r1.1 $s" 0 "1.1\nnew delta 1.1.1.1\n1 lines\n" \
	IGNORE
docommand b6 "${unget} $s" 0 "1.1.1.1\n" ""

remove $s $g $p




###
### Cleanup and exit.
###
rm -rf test 
remove foo $s $g $p command.log

success

