#! /bin/sh
# basic.sh:  Testing for the basic operation of "delta".

# Import common functions & definitions.
. ../common/test-common

remove command.log log log.stdout log.stderr
mkdir test 2>/dev/null

# Create the input files.
cat > base <<EOF
This is a test file containing nothing interesting.
EOF
for i in 1 2 3 4 5 6
do 
    cat base 			   > test/passwd.$i
    echo "This is file number" $i >> test/passwd.$i
done 

s=test/s.passwd

remove base test/[xz].*
remove test/[spx].passwd
remove passwd

#
# Create an SCCS file with several branches to work on.
# We generally ignore stderr output since we produce "Warning: no id keywords"
# more often than "real" SCCS.
#
docommand B1 "${admin} -itest/passwd.1 $s" 0 \
    ""                                              IGNORE
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
remove passwd command.log
success

