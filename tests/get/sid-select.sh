#! /bin/sh
# sid-select.sh:  Do we select the correct SIDs?

# Import common functions & definitions.
. test-common


# Get a test file...
uudecode < testfile.uue || miscarry could not extract test file.


get-expect () {
sid_expected=$1
shift
gotten=$(${get} -g $* 2>/dev/null | tail -1)
test x$sid_expected = x$gotten || {
    fail got wrong version  - got $gotten, expected $sid_expected 
    }
}

# Do various forms of get on the file and make sure we get the right SID.
get-expect 1.1        -r1.1           s.testfile
get-expect 1.2        -r1.2           s.testfile
get-expect 1.3        -r1.3           s.testfile
get-expect 1.4        -r1.4           s.testfile
get-expect 1.5        -r1.5           s.testfile
get-expect 1.5        -r1             s.testfile
get-expect 1.3.1.1    -r1.3.1.1       s.testfile
get-expect 1.3.1.1    -r1.3.1         s.testfile
get-expect 2.1        -r2.1           s.testfile
get-expect 2.1        -r2             s.testfile
get-expect 2.1        ""              s.testfile
get-expect 2.1        -r3             s.testfile
get-expect 2.1        -r9000          s.testfile

${get}                -r3.1           s.testfile 2>/dev/null && {
    fail 1 - should fail - no such SID 
}

${get}		      -r3.1.1        s.testfile 2>/dev/null && {
    fail 2 - should fail - no such SID
 }

remove s.testfile
success
