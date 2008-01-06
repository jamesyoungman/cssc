#! /bin/sh

# Import common functions & definitions.
. ../common/test-common

g=keys.txt
s=s.$g
l=l.$g
remove $s $g $l
../../testutils/uu_decode --decode < keys.uue || miscarry could not extract test file.


# Check the basic function of the -L option.
docommand L1 "${get} -L s.keys.txt" 0 \
"   1.2\t97/10/25 23:04:58 james\n\tchanged the one and only line\n
   1.1\t97/10/25 23:04:20 james\n\tdate and time created 97/10/25 23:04:20 by james\n1.2\n1 lines\n"
remove $g

# Check that -lp does the same thing.
docommand L1p "${get} -lp s.keys.txt" 0 \
"   1.2\t97/10/25 23:04:58 james\n\tchanged the one and only line\n
   1.1\t97/10/25 23:04:20 james\n\tdate and time created 97/10/25 23:04:20 by james\n1.2\n1 lines\n"
remove $g

# Check that the s-file header is in the correct position with respect to the 
# delta summary
docommand L2 "${get} -L s.keys.txt s.keys.txt" 0 \
"\n$s:\n   1.2\t97/10/25 23:04:58 james\n\tchanged the one and only line\n
   1.1\t97/10/25 23:04:20 james\n\tdate and time created 97/10/25 23:04:20 by james\n1.2\n1 lines\n\n$s:\n   1.2\t97/10/25 23:04:58 james\n\tchanged the one and only line\n
   1.1\t97/10/25 23:04:20 james\n\tdate and time created 97/10/25 23:04:20 by james\n1.2\n1 lines\n" ""
remove $g

# Check that the delta summary is sent to stderr if -p is given (and that 
# the body goes to stdout)..
docommand L3 "${get} -p -k -L s.keys.txt" 0 \
"1.2 %I%\n" \
"   1.2\t97/10/25 23:04:58 james\n\tchanged the one and only line\n\n   1.1\t97/10/25 23:04:20 james\n\tdate and time created 97/10/25 23:04:20 by james\n1.2\n1 lines\n"
remove $g


# Generate an l-file ...
docommand o1a "${get} -l s.keys.txt" 0 "1.2\n1 lines\n" ""

# ... and check its contents.
docommand o1b "cat $l" 0 "   1.2\t97/10/25 23:04:58 james\n\tchanged the one and only line\n\n   1.1\t97/10/25 23:04:20 james\n\tdate and time created 97/10/25 23:04:20 by james\n" ""
remove $g $l


remove $s $g $l
success
