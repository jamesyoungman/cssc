#! /bin/sh

# admin-hz.sh:  Tests for the -h and -z options of "admin".

# Import common functions & definitions.
. ../common/test-common

g=new.txt
s=s.$g
p=p.$g
s2=s.spare
remove foo $s $g $p [zx].$g $s2

###
### Tests for the 'v' flag; see also init-mrs.sh.
###

# Create SCCS file
echo 'hello from %M%' >foo

docommand c1 "${admin} -ifoo $s" 0 "" ""

# Make sure the checksum is checked as correct.
docommand c2 "${admin} -h $s" 0 "" ""

# Now, create a copy with a changed checksum, but no other 
# differences.
docommand c3 " (sed -e '1y/0123456789/9876453210/' <$s >$s2) " 0 "" ""

# Check that we think that the checksum of the file is wrong.
docommand c4 "${admin} -h $s2" 1 "" "IGNORE"

# Fix the checksum.
docommand c5 "${admin} -z $s2" 0 "" ""

# Check that we are happy again.
docommand c6 "${admin} -h $s2" 0 "" ""

# Make sure the files are again identical.
docommand c7 "diff $s $s2" 0 "" "IGNORE"


###
### Cleanup and exit.
###
rm -rf test 
remove foo $s $g $p [zx].$g command.log $s2

success

