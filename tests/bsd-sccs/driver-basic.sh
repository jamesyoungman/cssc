#! /bin/sh
# driver-basic.sh:  Testing for the basic operation of the BSD wrapper "sccs".
#                   We test each of the subcommands.

# Import common functions & definitions.
. ../common/test-common

remove command.log log log.stdout log.stderr SCCS
mkdir SCCS 2>/dev/null

g=tfile s=SCCS/s.$g p=SCCS/p.$g x=SCCS/x.$g z=SCCS/z.$g
remove $s $p $g $x $z

echo "Using the driver program ${sccs}"


# Create the input file.
cat > $g <<EOF
This is a test file containing nothing interesting.
EOF

# Create the s-file the traditional way...
docommand a1 "${sccs} admin -i$g $s" 0 \
    ""                                              IGNORE
docommand a2 "test -f $s" 0 "" ""
remove $s

docommand a3 "${sccs} enter $g" 0 \
    "\n$g:\n"                                        IGNORE
docommand a4 "test -f $s"  0 "" ""

# Check the backup file still exists.
docommand a5 "test -f ,$g" 0 "" ""
remove ,$g



success



docommand a2 "test -f $s" 0 "" ""


docommand B2 "${get} -e $s" 0 \
    "1.1\nnew delta 1.2\n2 lines\n"                 IGNORE
cp test/passwd.2 passwd
docommand B3 "${delta} -y\"\" $s" 0 \
    "1.2\n1 inserted\n1 deleted\n1 unchanged\n"     IGNORE
docommand B4 "${get} -e $s"  0 \
    "1.2\nnew delta 1.3\n2 lines\n"                 IGNORE
cp test/passwd.3 passwd
docommand B5 "${delta} -y'' $s" 0 \
    "1.3\n1 inserted\n1 deleted\n1 unchanged\n"     IGNORE
docommand B6 "${get} -e $s" 0 \
    "1.3\nnew delta 1.4\n2 lines\n"                 IGNORE
cp test/passwd.4 passwd
docommand B7 "${delta} -y'' $s" 0 \
    "1.4\n1 inserted\n1 deleted\n1 unchanged\n"     IGNORE
docommand B8 "${get} -e $s" 0 \
    "1.4\nnew delta 1.5\n2 lines\n"                 IGNORE
cp test/passwd.5 passwd
docommand B9 "${delta} -y'' $s" 0 \
    "1.5\n1 inserted\n1 deleted\n1 unchanged\n"     IGNORE
docommand B10 "${get} -e -r1.3 $s" 0 \
    "1.3\nnew delta 1.3.1.1\n2 lines\n"             IGNORE
cp test/passwd.6 passwd
docommand B11 "${delta} -y'' $s" 0 \
    "1.3.1.1\n1 inserted\n1 deleted\n1 unchanged\n" IGNORE

rm -rf test
remove passwd command.log $s $g $x $z $p SCCS
success

