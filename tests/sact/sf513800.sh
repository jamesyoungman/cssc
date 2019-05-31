#! /bin/sh

# sf513800.sh:  Tests relating to SOurceForge bug 513800

# Import common functions & definitions.
. ../common/test-common

g=foo
s=s.$g
p=p.$g

remove $s $p $g

# Set up the test input files. 
cp sf513800_s s.foo || miscarry 'could not set up test input s.foo'
cp sf513800_p p.foo || miscarry 'could not set up test input p.foo'

docommand s1 "${vg_sact} $s" 0 IGNORE IGNORE 

remove $s $p $g
success
