#! /bin/sh
# errorcases.sh:  Testing for various error cases
#

# Import common functions & definitions.
. ../common/test-common




remove 


g=foo
s=s.$g
p=p.$g
z=z.$g
q=q.$g
d=d.$g
x=x.$g
files="$g $s $p $z ${g}_1 ${g}_2 $g $q $d $x command.log log log.stdout log.stderr"
remove $files


# Create the input files.
echo foo > $g

docommand de1 "${admin} -n -i$g $s" 0 IGNORE IGNORE
remove $g
docommand de2 "${get} -s -p $s" 0 "foo\n" IGNORE
docommand de3 "${get} -s -p -r1.1 $s" 0 "foo\n" IGNORE

# Attempt to get a nonexistent SID should fail. 
docommand de4 "${get} -r1.2 $s" 1 "" IGNORE

# Attempt to get an invalid SID should fail (we try several)
docommand de5 "${get}  -r2a $s"  2 "" IGNORE
docommand de6 "${get}  -r2_3 $s" 2 "" IGNORE

# Make a branch for later use
docommand de7 "${get} -e $s" 0 "1.1\nnew delta 1.2\n1 lines\n" IGNORE
docommand de8 "${delta} -yNoComment $s" 0 IGNORE IGNORE
docommand de9 "${get} -e -r1.1 $s" 0 "1.1\nnew delta 1.1.1.1\n1 lines\n" IGNORE
docommand de10 "${delta} -yNoComment $s" 0 IGNORE IGNORE

# Now get 1.1.1.1 but including the change for 1.2.
docommand de11 "${get} -r1.1.1.1 -i1.2 $s" 0 "Included:
1.2
1.1.1.1\n1 lines
" IGNORE

# The next is trhe case we really want to test - trying to include an invalid
# SID.  We try several ways. 
docommand de12 "${get} -r1.1.1.1 -i1.2a $s"      2 "" IGNORE
docommand de13 "${get} -r1.1.1.1 -i.1   $s"      2 "" IGNORE
docommand de14 "${get} -r1.1.1.1 -i1.1.1.1.1 $s" 2 "" IGNORE

# Now trying to exclude an invalid SID.  We try several ways. 
docommand de15 "${get} -r1.1.1.1 -x1.2a $s"      2 "" IGNORE
docommand de16 "${get} -r1.1.1.1 -x.1   $s"      2 "" IGNORE
docommand de17 "${get} -r1.1.1.1 -x1.1.1.1.1 $s" 2 "" IGNORE


remove ${files}
success

