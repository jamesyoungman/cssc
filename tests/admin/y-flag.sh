#! /bin/sh

# y-flag.sh:  Testing for the 'y' flag for admin (admin -fy).

# Import common functions & definitions.
. ../common/test-common

# Determine if we are testing CSSC or the real thing.
. ../common/real-thing

g=bar
s=s.${g}
z=z.${g}

remove $s $g $z foo command.log last.command core 
remove expected.stderr got.stderr expected.stdout got.stdout

# Figure out if we ahous expect the thing to work.
if admin -n -i/dev/null -fyM ${s} >/dev/null 2>&1 || $TESTING_CSSC
then
    echo "We are testing an SCCS implementation that supports the y flag.  Good."
else
    echo "WARNING: some test have been skipped since I think that ${admin} does not support the 'y' flag."
    remove $s $g $z foo command.log last.command core 
    remove expected.stderr got.stderr expected.stdout got.stdout
    success
    exit 0
fi


remove foo
cat > foo <<EOF 
 1 M %M%
 2 R %R%
 3 L %L%
 4 B %B%
 5 S %S%
 6 Y %Y%
 7 F %F%
 8 Q %Q% 
 9 C %C%
10 C %C%
11 Z %Z%
12 W %W%
EOF
test -r foo || miscarry cannot create file foo.

docommand Y1 "${admin} -ifoo ${s}" 0 "" IGNORE
remove foo


# docommand A2 "${admin} -dy $s" 0 IGNORE IGNORE

# default situation is that everything is expanded.
docommand Y2 "${get} -p -r1.1 ${s}" 0 "\
 1 M bar
 2 R 1
 3 L 1
 4 B 0
 5 S 0
 6 Y 
 7 F s.bar
 8 Q  
 9 C 9
10 C 10
11 Z @(#)
12 W @(#)bar	1.1
" IGNORE


docommand YMa "${admin} -fyM ${s}" 0 "" IGNORE
docommand YMg "${get} -p -r1.1 ${s}" 0 "\
 1 M bar\n 2 R %R%\n 3 L %L%\n 4 B %B%\n 5 S %S%\n 6 Y %Y%
 7 F %F%\n 8 Q %Q% \n 9 C %C%\n10 C %C%\n11 Z %Z%\n12 W %W%
" IGNORE

# docommand Y_a "${admin} -fy_ ${s}" 0 "" IGNORE
# docommand Y_g "${get} -p -r1.1 ${s}" 0 "\
#  1 M %M%\n 2 R %R%\n 3 L %L%\n 4 B %B%\n 5 S %S%\n 6 Y %Y%
#  7 F %F%\n 8 Q %Q% \n 9 C %C%\n10 C %C%\n11 Z %Z%\n12 W %W%
# " IGNORE


docommand YRa "${admin} -fyR ${s}" 0 "" IGNORE
docommand YRg "${get} -p -r1.1 ${s}" 0 "\
 1 M %M%\n 2 R 1\n 3 L %L%\n 4 B %B%\n 5 S %S%\n 6 Y %Y%
 7 F %F%\n 8 Q %Q% \n 9 C %C%\n10 C %C%\n11 Z %Z%\n12 W %W%
" IGNORE


docommand YLa "${admin} -fyL ${s}" 0 "" IGNORE
docommand YLg "${get} -p -r1.1 ${s}" 0 "\
 1 M %M%\n 2 R %R%\n 3 L 1\n 4 B %B%\n 5 S %S%\n 6 Y %Y%
 7 F %F%\n 8 Q %Q% \n 9 C %C%\n10 C %C%\n11 Z %Z%\n12 W %W%
" IGNORE

docommand YBa "${admin} -fyB ${s}" 0 "" IGNORE
docommand YBg "${get} -p -r1.1 ${s}" 0 "\
 1 M %M%\n 2 R %R%\n 3 L %L%\n 4 B 0\n 5 S %S%\n 6 Y %Y%
 7 F %F%\n 8 Q %Q% \n 9 C %C%\n10 C %C%\n11 Z %Z%\n12 W %W%
" IGNORE

docommand YSa "${admin} -fyS ${s}" 0 "" IGNORE
docommand YSg "${get} -p -r1.1 ${s}" 0 "\
 1 M %M%\n 2 R %R%\n 3 L %L%\n 4 B %B%\n 5 S 0\n 6 Y %Y%
 7 F %F%\n 8 Q %Q% \n 9 C %C%\n10 C %C%\n11 Z %Z%\n12 W %W%
" IGNORE

docommand YYa "${admin} -fyY ${s}" 0 "" IGNORE
docommand YYg "${get} -p -r1.1 ${s}" 0 "\
 1 M %M%\n 2 R %R%\n 3 L %L%\n 4 B %B%\n 5 S %S%\n 6 Y 
 7 F %F%\n 8 Q %Q% \n 9 C %C%\n10 C %C%\n11 Z %Z%\n12 W %W%
" IGNORE


docommand YFa "${admin} -fyF ${s}" 0 "" IGNORE
docommand YFg "${get} -p -r1.1 ${s}" 0 "\
 1 M %M%\n 2 R %R%\n 3 L %L%\n 4 B %B%\n 5 S %S%\n 6 Y %Y%
 7 F s.bar\n 8 Q %Q% \n 9 C %C%\n10 C %C%\n11 Z %Z%\n12 W %W%
" IGNORE

docommand YQa "${admin} -fyQ ${s}" 0 "" IGNORE
docommand YQg "${get} -p -r1.1 ${s}" 0 "\
 1 M %M%\n 2 R %R%\n 3 L %L%\n 4 B %B%\n 5 S %S%\n 6 Y %Y%
 7 F %F%\n 8 Q  \n 9 C %C%\n10 C %C%\n11 Z %Z%\n12 W %W%
" IGNORE


docommand YCa "${admin} -fyC ${s}" 0 "" IGNORE
docommand YCg "${get} -p -r1.1 ${s}" 0 "\
 1 M %M%\n 2 R %R%\n 3 L %L%\n 4 B %B%\n 5 S %S%\n 6 Y %Y%
 7 F %F%\n 8 Q %Q% \n 9 C 9\n10 C 10\n11 Z %Z%\n12 W %W%
" IGNORE


docommand YZa "${admin} -fyZ ${s}" 0 "" IGNORE
docommand YZg "${get} -p -r1.1 ${s}" 0 "\
 1 M %M%\n 2 R %R%\n 3 L %L%\n 4 B %B%\n 5 S %S%\n 6 Y %Y%
 7 F %F%\n 8 Q %Q% \n 9 C %C%\n10 C %C%\n11 Z @(#)\n12 W %W%
" IGNORE

docommand YWa "${admin} -fyW ${s}" 0 "" IGNORE
docommand YWg "${get} -p -r1.1 ${s}" 0 "\
 1 M %M%\n 2 R %R%\n 3 L %L%\n 4 B %B%\n 5 S %S%\n 6 Y %Y%
 7 F %F%\n 8 Q %Q% \n 9 C %C%\n10 C %C%\n11 Z %Z%\n12 W @(#)bar	1.1
" IGNORE

docommand YCWa "${admin} -fyW,C ${s}" 0 "" IGNORE
docommand YCWg "${get} -p -r1.1 ${s}" 0 "\
 1 M %M%\n 2 R %R%\n 3 L %L%\n 4 B %B%\n 5 S %S%\n 6 Y %Y%
 7 F %F%\n 8 Q %Q% \n 9 C 9\n10 C 10\n11 Z %Z%\n12 W @(#)bar	1.1
" IGNORE


# Now, testing for %A% and %I%
remove ${g} ${s}


remove foo
cat > foo <<EOF 
 1 %Z%%Y% %M% %I%%Z%
 2 %A%
EOF
test -r foo || miscarry cannot create file foo.

docommand YA1 "${admin} -ifoo ${s}" 0 "" IGNORE
remove foo

docommand YA2 "${admin} -dy ${s}" 0 "" IGNORE
docommand YA3 "${get} -p -r1.1 ${s}" 0 "\
 1 @(#) bar 1.1@(#)
 2 @(#) bar 1.1@(#)
" IGNORE

# Disable expansion of %Z% and %I%, and check that it is still expanded in 
# %A%.
docommand YA4 "${admin} -fyA,M ${s}" 0 "" IGNORE
docommand YA5 "${get} -p -r1.1 ${s}" 0 "\
 1 %Z%%Y% bar %I%%Z%
 2 @(#) bar 1.1@(#)
" IGNORE

# Disable M as well and check again.
# Disable expansion of %Z% and %I%, and check that it is still expanded in 
# %A%.
docommand YA6 "${admin} -fyA ${s}" 0 "" IGNORE
docommand YA7 "${get} -p -r1.1 ${s}" 0 "\
 1 %Z%%Y% %M% %I%%Z%
 2 @(#) bar 1.1@(#)
" IGNORE


remove $s $g $z foo command.log last.command core 
remove expected.stderr got.stderr expected.stdout got.stdout
success
