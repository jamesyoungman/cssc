#! /bin/sh

# doubleinc.sh: tests for deltas that include other deltas.

# Copyright (C) 1999, Free Software Foundation.
# 
# This file is part of GNU CSSC; it may be distributed under the 
# terms of the GNU GPL, version 2 or later.  Please see the file
# COPYING for more information.


# Import common functions & definitions.
. ../common/test-common

# $ get -s -m -p -k s.tricky
# 1.1     version 1.1
# 1.2     version 1.2
# 1.9     version 1.2.1.2
# 1.3     version 1.3
# 1.9     version 1.3.1.3
# 1.4     version 1.4
# 1.5     version 1.5
# 1.6     version 1.6
# 1.7     version 1.7
# 1.8     version 1.8
# 1.9     version 1.9


# set -e
# set -x 

# Create a branch, which explicitly includes one of its own ancestors,
# which should be a benign thing to do.  We do this with a fresh file,
# in order to make things less compilicated.

g=simple
s=s.$g
p=p.$g
z=z.$g
x=x.$g
remove $g $s $p $z $x 

echo "v 1.1" > $g
docommand s1 "${admin} -i$g $s"   0 IGNORE IGNORE
remove $g
docommand s2 "${get} -e $s"       0 IGNORE IGNORE
echo "v 1.2" >> $g
docommand s3 "${delta} -y $s"     0 IGNORE IGNORE


docommand s4 "${get} -e -i1.2 $s" 0 IGNORE IGNORE
echo "v 1.3" >> $g
docommand s5 "${delta} -y $s"     0 IGNORE IGNORE

docommand s6 "${get} -p -k -s -r1.1 $s" 0 "v 1.1\n" ""
docommand s7 "${get} -p -k -s -r1.2 $s" 0 "v 1.1\nv 1.2\n" ""
docommand s8 "${get} -p -k -s -r1.3 $s" 0 "v 1.1\nv 1.2\nv 1.3\n" ""


remove $g $s $p $z $x





g=tricky
s=s.$g
p=p.$g
z=z.$g
x=x.$g
remove $g $s $p $z $x 


release=1
echo "version 1.1" > $g
docommand a1 "${admin} -i$g $s" 0 IGNORE IGNORE
remove $g


# Create the trunk.
echo_nonl "Creating the trunk..."
for level in 1 2 3 4 5 6 7 
do
    nextlevel=`expr $level + 1`
    
    sid=$release.$level
    nextsid=$release.$nextlevel
    echo_nonl "$nextsid "
    docommand --silent t$level "${get} -e -r$sid -s $s" 0 IGNORE IGNORE
    echo "version $nextsid" >> $g || miscarry failed to create trunk revision
    docommand --silent d$level "${delta} -y -s $s" 0 IGNORE IGNORE
done
echo done

# Create the 1.2.1.1 and branch.
#echo_nonl "Creating the 1.$b branch... "
#echo_nonl "1.2.1.1 "

docommand b1 "${get} -e -r1.2 -s $s" 0 IGNORE IGNORE
echo "version 1.2.1.1" >> $g
docommand b2 "${delta} -y -s $s"  0 IGNORE IGNORE


docommand c1 "${get} -r1.2.1.1 -s -p $s" 0 \
"version 1.1\nversion 1.2\nversion 1.2.1.1\n" IGNORE


#echo_nonl "1.2.1.2 "

docommand b3 "${get} -e -r1.2.1.1 -s $s"  0 IGNORE IGNORE
echo "version 1.2.1.2" >> $g
docommand b4 "${delta} -y -s $s"  0 IGNORE IGNORE

docommand c2 "${get} -r1.2.1.2 -s -p $s" 0 \
"version 1.1\nversion 1.2\nversion 1.2.1.1\nversion 1.2.1.2\n" IGNORE


#echo done

# Create the 1.3.1.1 branch.
#echo_nonl "Creating the 1.$b branch... "
#echo_nonl "1.3.1.1 "

docommand b5 "${get} -e -r1.3 -s $s" 0 IGNORE IGNORE
echo "version 1.3.1.1" >> $g
docommand b6 "${delta} -y -s $s"  0 IGNORE IGNORE

docommand c3 "${get} -r1.3.1.1 -p -s $s" 0 \
"version 1.1\nversion 1.2\nversion 1.3\nversion 1.3.1.1\n" IGNORE

#echo_nonl "1.3.1.2 "

docommand b7 "${get} -e -r1.3.1.1 -s $s"  0 IGNORE IGNORE
echo "version 1.3.1.2" >> $g
docommand b8 "${delta} -y -s $s"  0 IGNORE IGNORE

docommand c4 "${get} -r1.3.1.2 -s -p $s" 0 \
"version 1.1\nversion 1.2\nversion 1.3\nversion 1.3.1.1\nversion 1.3.1.2\n" \
IGNORE

#echo done



# Get the head of the second branch, but include the head of the first.
docommand i1 "${get} -p -s -r1.3.1.2 -i1.2.1.2 $s" 0 \
"version 1.1\nversion 1.2\nversion 1.2.1.2\nversion 1.3\nversion 1.3.1.1\nversion 1.3.1.2\n" IGNORE


# Create a new head for the second branch; include the head of the first.
docommand h1 "${get} -e -r1.3.1.2 -i1.2.1.2 $s"  0 IGNORE IGNORE
echo "version 1.3.1.3" >> $g
docommand h2 "${delta} -y -s $s"  0 IGNORE IGNORE

# Make sure the included delta is marked as included.
docommand p1 "${prs} -d:Dn: -r1.3.1.3 $s" 0 "10\n" ""

# Get 1.8, but include the head of the second branch.
docommand i2 "${get} -p -s -i1.3.1.3 $s" 0 \
"version 1.1\nversion 1.2\nversion 1.2.1.2\nversion 1.3\nversion 1.3.1.3\nversion 1.4\nversion 1.5\nversion 1.6\nversion 1.7\nversion 1.8\n" IGNORE


# Update the head of the trunk; include the head of the second branh.
docommand h3 "${get} -e  -i1.3.1.3 $s"  0 IGNORE IGNORE
echo "version 1.9" >> $g
docommand h4 "${delta} -y -s $s"  0 IGNORE IGNORE



# OK, now comes the interesting bit.  We test the result.

# First, we get a revision from the trunk that doesn't explicitly
# include any deltas and then do something similar or the branches, to 
# make sure that all the basic functionality works OK.

docommand g1 "${get} -p -k -s -r1.1 $s" 0 "version 1.1\n" ""
docommand g2 "${get} -p -k -s -r1.8 $s" 0 \
"version 1.1\nversion 1.2\nversion 1.3\nversion 1.4\nversion 1.5\nversion 1.6\nversion 1.7\nversion 1.8\n" ""

docommand g3 "${get} -p -k -s -r1.2.1.2 $s" 0 \
"version 1.1\nversion 1.2\nversion 1.2.1.1\nversion 1.2.1.2\n" ""
docommand g4 "${get} -p -k -s -r1.3.1.2 $s" 0 \
"version 1.1\nversion 1.2\nversion 1.3\nversion 1.3.1.1\nversion 1.3.1.2\n" ""


# Now we retrieve the top revision of the 1.3.1 branch, which includes 
# version 1.2.1.2
docommand g5 "${get} -p -k -s -r1.3.1.3 $s" 0 \
"version 1.1\nversion 1.2\nversion 1.2.1.2\nversion 1.3\nversion 1.3.1.1\nversion 1.3.1.2\nversion 1.3.1.3\n" ""

# Then we retrieve version 1.9, which includes version 1.3.1.3.
docommand g6 "${get} -p -k -s -r1.9 $s" 0 \
"version 1.1\nversion 1.2\nversion 1.2.1.2\nversion 1.3\nversion 1.3.1.3\nversion 1.4\nversion 1.5\nversion 1.6\nversion 1.7\nversion 1.8\nversion 1.9\n" ""


# Make a trunk revision, and see if the same thing applies to the 
# new trunk head.
docommand g7 "${get} -e $s"  0 IGNORE IGNORE
echo "version 1.10" >> $g
docommand g8 "${delta} -y -s $s"  0 IGNORE IGNORE
docommand g9 "${get} -p -k -s -r1.10 $s" 0 \
"version 1.1\nversion 1.2\nversion 1.2.1.2\nversion 1.3\nversion 1.3.1.3\nversion 1.4\nversion 1.5\nversion 1.6\nversion 1.7\nversion 1.8\nversion 1.9\nversion 1.10\n" ""


#cp $s $s.LAST

remove $g $s $p $z $x 
success

