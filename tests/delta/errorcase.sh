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
files="$g $s $p $z ${g}_1 ${g}_2 $g"

remove $files

append() {
   f="$1"
   shift
   echo  "$@" >> "$f" || miscarry "Could not append a line to $1" 
}


if wrong_group=`../../testutils/user foreigngroup`
then
    miscarry "could not select the name of a group to which you do not belong"
fi
# echo "You do not belong to group number" $wrong_group


# Create the SCCS file - and make sure that delta can be made to work at all.
docommand E1 "${admin} -n $s" 0 IGNORE IGNORE 
docommand E2 "${get} -e $s"   0 IGNORE IGNORE 
append $g "test data"
docommand E3 "${delta} -yNoComment $s"   0 IGNORE IGNORE 

# Now set up the authorised groups list.   
docommand E4 "${get} -e $s"   0 IGNORE IGNORE 
append $g "more test data"
docommand E5 "${admin} -a${wrong_group} $s" 0 IGNORE IGNORE
docommand E6 "${delta} -yNoComment $s"   1 IGNORE IGNORE 

# Remove the authorised group list; check-in should now work
docommand E7 "${admin} -e${wrong_group} $s" 0 IGNORE IGNORE
docommand E8 "${delta} -yNoComment $s"   0 IGNORE IGNORE 


remove $files 
success

