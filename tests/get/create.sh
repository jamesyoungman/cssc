#! /bin/sh
# sid-select.sh:  Do we select the correct SIDs?

# Import common functions & definitions.
. test-common

remove log log.stdout log.stderr
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
#
(
${admin} -itest/passwd.1 test/s.passwd
${get} -e -g test/s.passwd
cp test/passwd.2 passwd
${delta} -Y test/s.passwd
${get} -e -g test/s.passwd
cp test/passwd.3 passwd
${delta} -Y test/s.passwd
${get} -e -g -x1.2 test/s.passwd
cp test/passwd.4 passwd
${delta} -Y test/s.passwd
${get} -e -g test/s.passwd
cp test/passwd.5 passwd
${delta} -Y test/s.passwd
${get} -e -g -r1.3 test/s.passwd
cp test/passwd.6 passwd
${delta} -Y test/s.passwd
${get} -r1.1 -p test/s.passwd > test/passwd.m1
${DIFF} test/passwd.1 test/passwd.m1
${get} -r1.2 -p test/s.passwd > test/passwd.m2
${DIFF} test/passwd.2 test/passwd.m2
${get} -r1.3 -p test/s.passwd > test/passwd.m3
${DIFF} test/passwd.3 test/passwd.m3
${get} -r1.4 -p test/s.passwd > test/passwd.m4
${DIFF} test/passwd.4 test/passwd.m4
${get} -r1.5 -p test/s.passwd > test/passwd.m5
${DIFF} test/passwd.5 test/passwd.m5
${get} -r1.3.1.1 -p test/s.passwd > test/passwd.m6
${DIFF} test/passwd.6 test/passwd.m6
) > log.stdout 2>log.stderr

remove expected

cat >expected <<EOF

test/s.passwd
1.1
new delta 1.2

test/s.passwd
1.2
new delta 1.3

test/s.passwd
Excluded: 1.2
1.3
new delta 1.4

test/s.passwd
1.4
new delta 1.5

test/s.passwd
1.3
new delta 1.3.1.1
EOF

diff expected log.stdout || fail output format error
remove expected

cat >expected <<EOF
test/s.passwd: Warning: No id keywords.
test/s.passwd: Warning: No id keywords.
passwd: Warning: No id keywords.
test/s.passwd: Warning: No id keywords.
passwd: Warning: No id keywords.
test/s.passwd: Warning: No id keywords.
passwd: Warning: No id keywords.
test/s.passwd: Warning: No id keywords.
passwd: Warning: No id keywords.
test/s.passwd: Warning: No id keywords.
passwd: Warning: No id keywords.
test/s.passwd: Warning: No id keywords.

test/s.passwd
1.1
2 lines
test/s.passwd: Warning: No id keywords.

test/s.passwd
1.2
2 lines
test/s.passwd: Warning: No id keywords.

test/s.passwd
1.3
2 lines
test/s.passwd: Warning: No id keywords.

test/s.passwd
1.4
2 lines
test/s.passwd: Warning: No id keywords.

test/s.passwd
1.5
2 lines
test/s.passwd: Warning: No id keywords.

test/s.passwd
1.3.1.1
2 lines
EOF

diff expected log.stderr || fail stderr output format error.

remove test expected log log.stdout log.stderr 

success
