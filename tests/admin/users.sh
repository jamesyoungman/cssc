#! /bin/sh

# users.sh:  Testing for -a and -e options of admin.

# Import common functions & definitions.
. ../common/test-common

g=bar
s=s.${g}
z=z.${g}

remove $s $g $z foo command.log last.command core 
remove expected.stderr got.stderr expected.stdout got.stdout

remove foo
echo '%M%' > foo
test `cat foo` = '%M%' || miscarry cannot create file foo.

docommand A1 "${admin} -ifoo ${s}" 0 "" IGNORE
remove foo

docommand A2 "${prt} -u $s" 0 "\ns.bar:\n\n Users allowed to make deltas -- \n\teveryone\n" ""

# Check the adding of users.
docommand A3 "${admin} -abashful ${s}" 0 "" ""
docommand A4 "${admin} -agrumpy  ${s}" 0 "" ""
docommand A5 "${admin} -asleepy  ${s}" 0 "" ""

docommand A6 "${prt} -u $s" 0 "\ns.bar:\n\n Users allowed to make deltas -- \n\tsleepy\n\tgrumpy\n\tbashful\n" ""


# Check the removal of users.
docommand A7 "${admin} -esleepy  ${s}" 0 "" ""
docommand A8 "${prt} -u $s" 0 "\ns.bar:\n\n Users allowed to make deltas -- \n\tgrumpy\n\tbashful\n" ""
docommand A9 "${admin} -ebashful  ${s}" 0 "" ""
docommand A10 "${prt} -u $s" 0 "\ns.bar:\n\n Users allowed to make deltas -- \n\tgrumpy\n" ""
docommand A11 "${admin} -egrumpy  ${s}" 0 "" ""
docommand A12 "${prt} -u $s" 0 "\ns.bar:\n\n Users allowed to make deltas -- \n\teveryone\n" ""



# Adding and removing a user in the same command should still
# result in the user being added.
docommand A13 "${admin} -asleepy -esleepy ${s}" 0 "" ""
docommand A14 "${prt} -u $s" 0 "\ns.bar:\n\n Users allowed to make deltas -- \n\tsleepy\n" ""


remove $s $g $z foo command.log last.command core 
remove expected.stderr got.stderr expected.stdout got.stdout


success

