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

docommand A2 "${admin} -abashful ${s}" 0 "" ""
docommand A3 "${admin} -agrumpy  ${s}" 0 "" ""
docommand A4 "${admin} -asleepy  ${s}" 0 "" ""

docommand A5 "${prt} -u $s" 0 "\ns.bar:\n\nUsers allowed to make deltas --\n\tsleepy\n\tgrumpy\n\tbashful\n" ""

remove $s $g $z foo command.log last.command core 
remove expected.stderr got.stderr expected.stdout got.stdout


success

