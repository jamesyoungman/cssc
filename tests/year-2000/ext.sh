#! /bin/sh

#################################################################
###          WARNING: this test is CSSC-specific!             ###
#################################################################

# ext.sh:       Testing for the century-specification
#               of CSSC.  This is an extension; other
#               SCCS implementations do not do this.

# Import common functions & definitions.
. ../common/test-common
. ../common/real-thing


s=s.y2k.txt

# All the commands in this file print timestamps.  Some
# implementations of SCCS convert the timestamp data to a time_t value
# as an intermediate representation.  Our test input file contains
# values at the extreme ends of the range.  Hence we set TZ to GMT to
# prevent overflows.  CSSC isn't subject to these overflows even on a
# 32-bit system, since it doesn't use a scalar intermediate value.
TZ=GMT
export TZ

brief='"-d:I: :D: :T:"'

r1_5="1.5 68/12/31 23:59:59\n" # 2068: the last year we have
r1_4="1.4 00/02/29 00:00:00\n" # Year 2000 is a leap year.
r1_3="1.3 00/01/01 00:00:00\n" # Just after the milennium
r1_2="1.2 99/12/31 23:59:59\n" # Just before the milennium
r1_1="1.1 69/01/01 00:00:00\n" # 1969: the earliest year we have

allrevs="${r1_5}${r1_4}${r1_3}${r1_2}${r1_1}"


if "$TESTING_CSSC"
then
    ## Tests for the century field.

    # Ask for exerything after the end of 1968.  Since the first
    # year we have int he s. file is 1969, we should get everything.
    docommand c1 "${vg_prs} ${brief} -l -c19681231235959  $s" 0 \
	"${allrevs}" ""

    # Ask for exerything before the end of 1968.  Since the first
    # year we have int he s. file is 1969, we should get NOTHING.
    docommand c2 "${vg_prs} ${brief} -e -c19681231235959  $s" 0 \
	"" ""

    # Ask for exerything before the end of 2069.  
    # We chould get everything.
    docommand c3 "${vg_prs} ${brief} -e -c20691231235959  $s" 0 \
	"${allrevs}" ""


else
    echo No testing done for century specifier.
fi


remove command.log
success
