#! /bin/sh
# errorcase.sh:  Testing for the various error cases for "delta".

# Import common functions & definitions.
. ../common/test-common

remove command.log log log.stdout log.stderr
mkdir test 2>/dev/null

g=foo
s=s.$g
p=p.$g
z=z.$g
q=q.$g
files="$g $s $p $z ${g}_1 ${g}_2 $g $q"

remove $files

append() {
   f="$1"
   shift
   echo  "$@" >> "$f" || miscarry "Could not append a line to $1" 
}


if wrong_group=`../../testutils/user foreigngroup`
then
    true
else
    miscarry "could not select the name of a group to which you do not belong"
fi
# echo "You do not belong to group number" $wrong_group


# Create the SCCS file - and make sure that delta can be made to work at all.
docommand E1 "${admin} -n $s" 0 IGNORE IGNORE 
docommand E2 "${get} -e $s"   0 IGNORE IGNORE 
append $g "test data"
docommand E3 "${delta} -yNoComment $s"   0 IGNORE IGNORE 

# Now set up the authorised groups list.   
docommand E4 "${admin} -a${wrong_group} $s" 0 IGNORE IGNORE

# cannot do get -e if you are not in the authorised user list.
docommand E5 "${get} -e $s"   1 IGNORE IGNORE 

# Momentarily zap the authorised user list so that "get -e" works.
docommand E6 "${admin} -e${wrong_group} $s" 0 IGNORE IGNORE
docommand E7 "${get} -e $s"   0 IGNORE IGNORE
docommand E8 "${admin} -a${wrong_group} $s" 0 IGNORE IGNORE

append $g "more test data"

# delta should still fail if we are not in the authorised user list
# (in other words the list is checked both by get -e and delta).
docommand E9 "${delta} -yNoComment $s"   1 IGNORE IGNORE

# Remove the authorised group list; check-in should now work
docommand E10 "${admin} -e${wrong_group} $s" 0 IGNORE IGNORE
docommand E11 "${delta} -yNoComment $s"   0 IGNORE IGNORE 


# Now, what if the authorised user list just excludes?
remove $s
if mygroup=`../../testutils/user group`
then
    true
else
    miscarry "could not determine group-id"
fi

if myname=`../../testutils/user name`
then
    true
else
    miscarry "could not determine user name"
fi

# Regular SCCS does not underatand the use of "!username" 
# to specifically exclude users.  Hence for compatibility 
# nor must we.
docommand E12 "${admin} -n $s"              0 IGNORE IGNORE 
docommand E13 "${admin} -a${mygroup} $s"    0 IGNORE IGNORE
docommand E14 "${admin} -a\!${myname} $s"   0 IGNORE IGNORE
docommand E15 "${get} -e $s"                0 IGNORE IGNORE
# this means that the above tests should succeed.

# Check - use of delta when a q-file already exists...
touch $q
test -r $q || miscarry "could not create file $q"
docommand E16 "${delta} -yNoComment $s" 1 IGNORE IGNORE
remove $q

# Unreadable g-file should also cause a failure. 
chmod 0 $g
docommand E17 "${delta} -yNoComment $s" 1 IGNORE IGNORE
chmod +r $g
docommand E18 "${delta} -yNoComment $s" 0 IGNORE IGNORE


remove $files 
success

