#! /bin/sh
# sid-select.sh:  Do we select the correct SIDs?

# Import common functions & definitions.
. ../common/test-common


# Get a test file...
s=s.testfile
remove $s
uudecode < testfile.uue || miscarry could not extract test file.

get-expect () {
label=$1         ; shift
sid_expected=$1  ; shift
docommand $label "${get} -g $*" 0 "$sid_expected\n" IGNORE
}

# Do various forms of get on the file and make sure we get the right SID.
get-expect X1  1.1        -r1.1      $s
get-expect X2  1.2        -r1.2      $s
get-expect X3  1.3        -r1.3      $s
get-expect X4  1.4        -r1.4      $s
get-expect X5  1.5        -r1.5      $s
get-expect X6  1.5        -r1        $s
get-expect X7  1.3.1.1    -r1.3.1.1  $s
get-expect X8  1.3.1.1    -r1.3.1    $s
get-expect X9  2.1        -r2.1      $s
get-expect X10 2.1        -r2        $s
get-expect X11 2.1        ""         $s
get-expect X12 2.1        -r3        $s
get-expect X13 2.1        -r9000     $s

docommand F1 "${get} -r3.1   s.testfile" 1 "" IGNORE
docommand F2 "${get} -r3.1.1 s.testfile" 1 "" IGNORE

remove $s
success
