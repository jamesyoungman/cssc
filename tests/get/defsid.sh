#! /bin/sh
# defsid.sh:  Tests for the "d" (default sid) flag.

# Import common functions & definitions.
. ../common/test-common

remove command.log 

g=brtest
s=s.$g
z=z.$g
x=x.$g
p=p.$g
remove [zxsp].$g $g

# Create the s. file and make sure it exists.
remove $g
echo "%M%" > $g
docommand d1 "$admin -n -i$g $s" 0 "" IGNORE
remove $g

# Create a second revision.
docommand d2 "$get -e $s" 0 "1.1\nnew delta 1.2\n1 lines\n" ""

# Check the file in to leave the branch in place.
docommand d3 "$delta -yNoComment $s" 0 "1.2\n0 inserted\n0 deleted\n1 unchanged\n" ""

# Make a branch at the same place, and check the resulting SID.
docommand d4 "$get -e -b -r1.1 $s" 0 "1.1\nnew delta 1.1.1.1\n1 lines\n" ""
docommand d5 "$delta -yNoComment $s" 0 "1.1.1.1\n0 inserted\n0 deleted\n1 unchanged\n" ""


# Set the default-sid flag.
docommand d6 "$admin -fd1.1 $s" 0 "" ""

# Make sure we select that SID.
docommand d7 "$get -g $s" 0 "1.1\n" ""


# Change the default-sid flag.
docommand d8 "$admin -fd1.1.1.1 $s" 0 "" ""

# Make sure we select that SID.
docommand d9 "$get -g $s" 0 "1.1.1.1\n" ""

# Delete the flag.
docommand d10 "$admin -dd $s" 0 "" ""

# Make sure we select the right SID now..
docommand d11 "$get -g $s" 0 "1.2\n" ""

remove [zxsp].$g $g
remove command.log
success

