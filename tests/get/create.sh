#! /bin/sh
# sid-select.sh:  Do we select the correct SIDs?

# Import common functions & definitions.
. test-common

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
remove base test/[xz].*
remove test/[spx].passwd


#
# Create an SCCS file with several branches to work on.
# We generally ignore stderr output since we produce "Warning: no id keywords"
# more often than "real" SCCS.
#
docommand L1 "${admin} -itest/passwd.1 test/s.passwd" 0 "" IGNORE
docommand L2 "${get} -e -g test/s.passwd"             0 "1.1\nnew delta 1.2\n" IGNORE
cp test/passwd.2 passwd
docommand L3 "${delta} -y\"\" test/s.passwd" 0 "1.2\n1 inserted\n1 deleted\n1 unchanged\n" IGNORE
docommand L4 "${get} -e -g test/s.passwd"  0 "1.2\nnew delta 1.3\n" IGNORE
cp test/passwd.3 passwd
docommand L5 "${delta} -y'' test/s.passwd" 0 "1.3\n1 inserted\n1 deleted\n1 unchanged\n" IGNORE
docommand L6 "${get} -e -g -x1.2 test/s.passwd" 0 "Excluded:\n1.2\n1.3\nnew delta 1.4\n" IGNORE
cp test/passwd.4 passwd
docommand L7 "${delta} -y'' test/s.passwd" 0 "1.4\n1 inserted\n2 deleted\n1 unchanged\n" IGNORE
docommand L8 "${get} -e -g test/s.passwd" 0 "1.4\nnew delta 1.5\n" IGNORE
cp test/passwd.5 passwd
docommand L9 "${delta} -y'' test/s.passwd" 0 "1.5\n1 inserted\n1 deleted\n1 unchanged\n" IGNORE
docommand L10 "${get} -e -g -r1.3 test/s.passwd" 0 "1.3\nnew delta 1.3.1.1\n" IGNORE
cp test/passwd.6 passwd
docommand L11 "${delta} -y'' test/s.passwd" 0 \
    "1.3.1.1\n1 inserted\n1 deleted\n1 unchanged\n" IGNORE

do_output L12 "${get} -r1.1 -p test/s.passwd"      0 test/passwd.1 IGNORE
do_output L13 "${get} -r1.2 -p test/s.passwd"      0 test/passwd.2 IGNORE
do_output L14 "${get} -r1.3 -p test/s.passwd"      0 test/passwd.3 IGNORE
do_output L15 "${get} -r1.4 -p test/s.passwd"      0 test/passwd.4 IGNORE
do_output L16 "${get} -r1.5 -p test/s.passwd"      0 test/passwd.5 IGNORE
do_output L17 "${get} -r1.3.1.1 -p test/s.passwd"  0 test/passwd.6 IGNORE

remove command.log
success

