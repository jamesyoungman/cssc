#! /bin/sh
# iterbasic.sh:  Testing for the basic operation of "delta"
#                which are checked as we go along.

# Import common functions & definitions.
. ../common/test-common

remove command.log log log.stdout log.stderr

g=iter
s=s.$g
p=p.$g
d=d.$g
x=x.$g
z=z.$g
remove $s $g $p $x $d $z

# Do some simple checks.
docommand I1 "${admin} -n $s" 0 "" IGNORE 
docommand I2 "${get} -s -k -p $s" 0 "" IGNORE
docommand I3 "${get} -e $s" 0 \
    "1.1\nnew delta 1.2\n0 lines\n"                 IGNORE

# Change nothing.  Ensure there is no change.
# We use -p, but diff should produce no ouput.    
docommand I4 "${delta} -p -y\"\" $s" 0 \
    "1.2\n0 inserted\n0 deleted\n0 unchanged\n"     IGNORE

# Now add a line at the end.
docommand I5 "${get} -e $s"  0 \
    "1.2\nnew delta 1.3\n0 lines\n"                 IGNORE
echo "hello" >> $g
echo "there" >> $g

docommand I6 "${delta} -y'' $s" 0 \
    "1.3\n2 inserted\n0 deleted\n0 unchanged\n"     IGNORE
docommand I7a "${get} -s -k -p $s" 0 "hello\nthere\n" IGNORE
docommand I7b "${get} -r1.1 -s -k -p $s" 0 "" IGNORE
docommand I7c "${get} -r1.2 -s -k -p $s" 0 "" IGNORE
docommand I7d "${get} -r1.3 -s -k -p $s" 0 "hello\nthere\n" IGNORE

# Now we delete a line.
docommand I8 "${get} -e $s"  0 \
    "1.3\nnew delta 1.4\n2 lines\n"                 IGNORE
remove $g    
echo "hello" > $g

#export CSSC_DEBUG_TRAVERSE=1

docommand I9 "${delta} -y'' -p $s" 0 \
    "1.4\n2d1\n< there\n0 inserted\n1 deleted\n1 unchanged\n"     IGNORE

#docommand I9 "${delta} -y'' $s" 0 \
#    "1.4\n0 inserted\n1 deleted\n1 unchanged\n"     IGNORE

docommand I10a "${get} -s -k -p -r1.1 $s" 0 "" IGNORE
docommand I10b "${get} -s -k -p -r1.2 $s" 0 "" IGNORE
docommand I10c "${get} -s -k -p -r1.3 $s" 0 "hello\nthere\n" IGNORE
docommand I10d "${get} -s -k -p -r1.4 $s" 0 "hello\n"        IGNORE

# And here we change a line.
docommand I11 "${get} -e $s"  0 \
    "1.4\nnew delta 1.5\n1 lines\n"                 IGNORE
remove $g    
echo "goodbye" > $g

docommand I12 "${delta} -y'' $s" 0 \
    "1.5\n1 inserted\n1 deleted\n0 unchanged\n"     IGNORE

docommand I13 "${get} -s -k -p $s" 0 "goodbye\n" IGNORE

remove $s $g $p $x $d $z
remove command.log
success

