#! /bin/sh
# basic2.sh:  Testing for the basic operation of "delta", but
#             in somewhat greater depth than basic.sh.

exit 0

# Import common functions & definitions.
. ../common/test-common

remove command.log log log.stdout log.stderr

# Create the input files.
remove base
cat > base <<EOF
This is a test file containing nothing interesting.
EOF

gbase=tfile

for i in 1 2 3 4 5 6
do 
    cat base 			   > $gbase.$i
    echo "This is file number" $i >> $gbase.$i
done 

g=tfile
s=s.$g
p=p.$g
z=z.$g
x=x.$g
rubbish="$x $z $p $x $g base"
remove $rubbish


#
# Create an SCCS file with several branches to work on.
# We generally ignore stderr output since we produce "Warning: no id keywords"
# more often than "real" SCCS.
#
remove $s
docommand C1 "${admin} -i$gbase.1 $s" 0 \
    ""                                              IGNORE
docommand C2 "${prt} -b $s" 0 \
"
s.tfile:

*** I 1
\tThis is a test file containing nothing interesting.
\tThis is file number 1
*** E 1\n" \
 IGNORE

docommand C3 "${get} -e $s" 0 \
    "1.1\nnew delta 1.2\n2 lines\n"                 IGNORE
cp $gbase.2 $g
docommand C4 "${delta} -y\"\" $s" 0 IGNORE IGNORE

    
docommand C5 "${prt} -b $s" 0 \
"
s.tfile:

*** I 1
\tThis is a test file containing nothing interesting.
*** D 2
\tThis is file number 1
*** E 1
*** E 2
*** I 2
\tThis is file number 2
*** E 2\n" \
 IGNORE

docommand C6 "${get} -e $s"  0 \
    "1.2\nnew delta 1.3\n2 lines\n"                 IGNORE
cp $gbase.3 $g
docommand C7 "${delta} -y'' $s" 0 IGNORE     IGNORE
docommand C8 "${prt} -b $s" 0 \
"
s.tfile:

*** I 1
\tThis is a test file containing nothing interesting.
*** D 2
*** D 3
\tThis is file number 1
*** E 1
*** E 2
*** I 2
\tThis is file number 2
*** E 2
*** E 3
*** I 3
\tThis is file number 3
*** E 3\n" \
 IGNORE



docommand C9 "${get} -e $s" 0 \
    "1.3\nnew delta 1.4\n2 lines\n"                 IGNORE
cp $gbase.4 $g
docommand C10 "${delta} -y'' $s" 0 \
    IGNORE     IGNORE
docommand C11 "${prt} -b $s" 0 \
"
s.tfile:

*** I 1
\tThis is a test file containing nothing interesting.
*** D 2
*** D 3
*** D 4
\tThis is file number 1
*** E 1
*** E 2
*** I 2
\tThis is file number 2
*** E 2
*** E 3
*** I 3
\tThis is file number 3
*** E 3
*** E 4
*** I 4
\tThis is file number 4
*** E 4\n" IGNORE

docommand C12 "${get} -e $s" 0 \
    "1.4\nnew delta 1.5\n2 lines\n"                 IGNORE
cp $gbase.5 $g
docommand C13 "${delta} -y'' $s" 0 \
    IGNORE     IGNORE
docommand C14 "${prt} -b $s" 0 \
"
s.tfile:

*** I 1
\tThis is a test file containing nothing interesting.
*** D 2
*** D 3
*** D 4
*** D 5
\tThis is file number 1
*** E 1
*** E 2
*** I 2
\tThis is file number 2
*** E 2
*** E 3
*** I 3
\tThis is file number 3
*** E 3
*** E 4
*** I 4
\tThis is file number 4
*** E 4
*** E 5
*** I 5
\tThis is file number 5
*** E 5\n" IGNORE


docommand C15 "${get} -e -r1.3 $s" 0 \
    "1.3\nnew delta 1.3.1.1\n2 lines\n"             IGNORE
cp $gbase.6 $g

# docommand C16 "${delta} -y'' $s" 0 \
#     "1.3.1.1\n1 inserted\n1 deleted\n1 unchanged\n" IGNORE

docommand C16 "${delta} -y'' $s" 0 IGNORE IGNORE
    
docommand C17 "${prt} -b $s" 0 \
"
s.tfile:

*** I 1
\tThis is a test file containing nothing interesting.
*** D 2
*** D 3
*** D 4
*** D 5
*** D 6
\tThis is file number 1
*** E 1
*** E 2
*** I 2
\tThis is file number 2
*** E 2
*** E 3
*** I 3
\tThis is file number 3
*** E 3
*** E 4
*** I 4
*** E 6
*** I 6
\tThis is file number 6
*** E 6
\tThis is file number 4
*** E 4
*** E 5
*** I 5
\tThis is file number 5
*** E 5\n" \
IGNORE 


rm -rf test
remove $g command.log $rubbish $s
success

