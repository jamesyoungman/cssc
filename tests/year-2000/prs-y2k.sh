#! /bin/sh


# prs-y2k.sh:  Testing for correct operation of prs
#               with regard to date issues.

# Import common functions & definitions.
. ../common/test-common


s=s.y2k.txt

brief='"-d:I: :D: :T:"'

r1_5="1.5 68/12/31 23:59:59\n" # 2068: the last year we have
r1_4="1.4 00/02/29 00:00:00\n" # Year 2000 is a leap year.
r1_3="1.3 00/01/01 00:00:00\n" # Just after the milennium
r1_2="1.2 99/12/31 23:59:59\n" # Just before the milennium
r1_1="1.1 69/01/01 00:00:00\n" # 1969: the earliest year we have

allrevs="${r1_5}${r1_4}${r1_3}${r1_2}${r1_1}"


## And now the tests.

## If we just specify -e without -c we should get all the revisions.
## Check that the dates are printed correctly.
docommand A1 "${prs} ${brief} -e $s" 0 "${allrevs}" ""

docommand t1 "${prs} ${brief} -e -c690101000000  $s" 0 \
    "${r1_1}" ""
docommand t2 "${prs} ${brief} -l -c690101000000  $s" 0 \
    "${allrevs}" ""

docommand t3 "${prs} ${brief} -l -c690101000001  $s" 0 \
    "${r1_5}${r1_4}${r1_3}${r1_2}" ""

docommand t4 "${prs} ${brief} -e -c991231235959  $s" 0 \
    "${r1_2}${r1_1}" ""

docommand t5 "${prs} ${brief} -l -c991231235959  $s" 0 \
    "${r1_5}${r1_4}${r1_3}${r1_2}" ""

docommand t6 "${prs} ${brief} -e -c000101000000  $s" 0 \
    "${r1_3}${r1_2}${r1_1}" ""
docommand t7 "${prs} ${brief} -l -c000101000000  $s" 0 \
    "${r1_5}${r1_4}${r1_3}" ""

docommand t8 "${prs} ${brief} -l -c000101000001  $s" 0 \
    "${r1_5}${r1_4}" ""

docommand t9 "${prs} ${brief} -e -c000229000000  $s" 0 \
    "${r1_4}${r1_3}${r1_2}${r1_1}" ""
docommand t10 "${prs} ${brief} -e -c000229000001  $s" 0 \
    "${r1_4}${r1_3}${r1_2}${r1_1}" ""
docommand t11 "${prs} ${brief} -l -c000229000000  $s" 0 \
    "${r1_5}${r1_4}" ""

docommand t12 "${prs} ${brief} -l -c681231235959  $s" 0 \
    "${r1_5}" ""


## Tests involving fields that take default values...

# Just giving the year should be equivalent to explicitly
# specifying the last second of that year.
docommand d1 "${prs} ${brief} -l -c99  $s" 0 \
    "${r1_5}${r1_4}${r1_3}${r1_2}" ""

docommand d2 "${prs} ${brief} -l -c0001  $s" 0 \
    "${r1_5}${r1_4}" ""

docommand d3 "${prs} ${brief} -l -c000228  $s" 0 \
    "${r1_5}${r1_4}" ""
docommand d4 "${prs} ${brief} -e -c68  $s" 0 \
    "${r1_5}${r1_4}${r1_3}${r1_2}${r1_1}" ""

docommand d5 "${prs} ${brief} -l -c68  $s" 0 \
    "${r1_5}" ""




remove command.log
success

 