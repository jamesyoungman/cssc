#! /bin/sh
# included.sh:  Tests for SourceForge bug number 111140.

# Import common functions & definitions.
. ../common/test-common


g=sf111140_testcase.txt
s=s.$g
x=x.$g 
z=z.$g
p=p.$g
u=sf111140_testcase.uue

remove $g $s $x $z $p

../../testutils/uu_decode --decode <$u || 
    miscarry could not uudecode file $u.



# If we check out version 1.16 of the provided file (in which 
# a trunk delta includes a delta that was on a trunk) we 
# should get the same body as recorded in the file sf111140.wtd.
# 
do_output s1 "${get} -r1.16 -p $s"      0 sf111140.wtd IGNORE

remove $g $s $x $z $p
success
