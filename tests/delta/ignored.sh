#! /bin/sh
# ignored.sh:  Testing for the -g option of "delta".

# Import common functions & definitions.
. ../common/test-common


# The manual pages for SCCS are not very explicit about the functionality
# of the -g option to delta.  In particular, it says nothing about how
# this interacts with the -i and -x options of "get -e".

# The tests in this directory exist primarily to be run on the reall
# SCCS suite in order to determine the properties of delta's -g
# option.

# I have been unable to determine what this flag does.  Rather than accept
# it and ignore it, we currently reject this option.

echo "No useful tests in this file."
exit 0



g=foo
s=s.$g
z=z.$g
x=x.$g
p=p.$g 
d=d.$g

remove $s $g $z $x $p $d


# Create an SCCS file.
cat > $g <<EOF
This is a test file.
It contains two lines in version 1.1.
EOF

docommand g1 "${admin} -i$g $s"    0 "" IGNORE
remove $g

# Check the file out for editing.
docommand g2 "${get} -e $s"      0 IGNORE IGNORE

# delete the contents.
remove $g
cat > $g <<EOF
This is a test file.
Version 1.2 also contains 2 lines, just like version 1.1.
EOF

# Check the result in (1.2).
docommand g3 "${delta} -y $s" 0 IGNORE IGNORE



# Check the file out for editing again; make a branch.
docommand g4a "${admin} -fb $s"      0 IGNORE IGNORE
docommand g4b "${get} -e -b -r1.2 $s"      0 IGNORE IGNORE

remove $g
cat > $g <<EOF
This is a test file.
The second line of version 1.2.1.1 says nothing in particular.
EOF

# Do nothing to the file, but use the -g option to ignore 1.1 and 1.2.
docommand g5 "${delta} -y $s" 0 IGNORE IGNORE



# Check the file out for editing again; make version 1.3.
docommand g6 "${get} -e $s"      0 IGNORE IGNORE
docommand g7 "${delta} -y -g1.2 $s"      0 IGNORE IGNORE




remove $s $g $z $x $p $d
success

