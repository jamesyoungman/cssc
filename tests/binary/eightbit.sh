#! /bin/sh
# eightbit.sh:  Testing for 8-bit clean operation 

# Import common functions & definitions.
. ../common/test-common

g=8bit.txt
p=p.$g
s=s.$g
x=x.$g
z=z.$g


cleanup() {
   remove command.log log log.stdout log.stderr
   remove $p $x $s $z $g
   remove passwd command.log last.command
   remove got.stdout expected.stdout got.stderr expected.stderr
   rm -rf test
}

cleanup

# At the moment, just create an SCCS file from 8-bit characters and
# make sure the checksum is OK.

# If the next line is incomprehensible to you, that's OK. 
# It contains ISO-8859-1 characters.  But the important thing
# is that they are outside the range 0...127.
remove $g
echo "garçon mañana áóäæèêëìñåòôùé" >$g
docommand a1 "${admin} -i$g $s" 0 IGNORE IGNORE
docommand a2 "${get} -p $s" 0 "garçon mañana áóäæèêëìñåòôùé\n" IGNORE

echo_nonl a3...
if uudecode s.umsp.uue
then
    echo passed
else
    miscarry uudecode failed.
fi

cmd="${get} -p s.umsp.txt" 
echo_nonl a4...
if ${cmd} > output 2>/dev/null
then
    echo passed
else
    fail "a4: $cmd failed"
fi

docommand a5 "cat output" 0 "garçon mañana áóäæèêëìñåòôùé\n" IGNORE
remove output s.umsp.txt

cleanup
success
