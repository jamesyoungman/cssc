#! /bin/sh

# y-flag.sh:  Testing for the 'y' flag for admin (admin -fy).
#
# The y flag determines which keywords get expanded.

# Import common functions & definitions.
. ../common/test-common

# Determine if we are testing CSSC or the real thing.
. ../common/real-thing

g=bar
s="s.${g}"
z="z.${g}"

remove "${s}" "${g}" "${z}" foo command.log last.command core
remove expected.stderr got.stderr expected.stdout got.stdout

# Figure out if we should expect the thing to work.
if ${admin} -n -i/dev/null -fyM "${s}" >/dev/null 2>&1 || ${TESTING_CSSC}
then
    test -e "${s}" || abandon_test_script "admin program '${admin}' silently did nothing"
    echo "We are testing an SCCS implementation that supports the y flag.  Good."
    remove "${s}"
else
    echo "WARNING: some test have been skipped since I think that ${admin} does not support the 'y' flag."
    remove "${s}" "${g}" "${z}" foo command.log last.command core
    remove expected.stderr got.stderr expected.stdout got.stdout
    success
fi


remove foo
copy y-flag-foo-initial inputs/foo.initial.txt foo
test -r foo || abandon_test_script cannot create file foo.
test -e "${s}" && abandon_test_script initial conditions were incorrectly set up

docommand Y1 "${admin} -ifoo ${s}" 0 "" IGNORE
remove foo

# default situation is that everything is expanded.
docommand --stdout_is_file Y2 "${vg_get} -p -r1.1 ${s}" 0 inputs/foo.kw-expansion.default.binary IGNORE

# Only expand the M flag.
docommand YMa "${vg_admin} -fyM ${s}" 0 "" IGNORE
docommand --stdout_is_file  YMg "${get} -p -r1.1 ${s}" 0 inputs/foo.kw-expansion.M-only.binary IGNORE

# Only expand the R flag.
docommand YRa "${vg_admin} -fyR ${s}" 0 "" IGNORE
docommand --stdout_is_file YRg "${get} -p -r1.1 ${s}" 0 inputs/foo.kw-expansion.R-only.binary IGNORE

# Only expand the L flag.
docommand YLa "${vg_admin} -fyL ${s}" 0 "" IGNORE
docommand --stdout_is_file YLg "${vg_get} -p -r1.1 ${s}" 0 inputs/foo.kw-expansion.L-only.binary IGNORE

# Only expand the B flag.
docommand YBa "${vg_admin} -fyB ${s}" 0 "" IGNORE
docommand --stdout_is_file YBg "${vg_get} -p -r1.1 ${s}" 0 inputs/foo.kw-expansion.B-only.binary IGNORE

# Only expand the S flag.
docommand YSa "${vg_admin} -fyS ${s}" 0 "" IGNORE
docommand --stdout_is_file YSg "${vg_get} -p -r1.1 ${s}" 0 inputs/foo.kw-expansion.S-only.binary IGNORE

# Only expand the Y flag.
docommand YYa "${vg_admin} -fyY ${s}" 0 "" IGNORE
docommand --stdout_is_file YYg "${vg_get} -p -r1.1 ${s}" 0 inputs/foo.kw-expansion.Y-only.binary IGNORE

# Only expand the F flag.
docommand YFa "${vg_admin} -fyF ${s}" 0 "" IGNORE
docommand --stdout_is_file YFg "${vg_get} -p -r1.1 ${s}" 0 inputs/foo.kw-expansion.F-only.binary IGNORE

# Only expand the Q flag.
docommand YQa "${vg_admin} -fyQ ${s}" 0 "" IGNORE
docommand --stdout_is_file YQg "${vg_get} -p -r1.1 ${s}" 0 inputs/foo.kw-expansion.Q-only.binary IGNORE

# Only expand the C flag.
docommand YCa "${vg_admin} -fyC ${s}" 0 "" IGNORE
docommand --stdout_is_file YCg "${vg_get} -p -r1.1 ${s}" 0 inputs/foo.kw-expansion.C-only.binary IGNORE

# Only expand the Z flag.
docommand YZa "${vg_admin} -fyZ ${s}" 0 "" IGNORE
docommand --stdout_is_file YZg "${vg_get} -p -r1.1 ${s}" 0 inputs/foo.kw-expansion.Z-only.binary IGNORE

# Only expand the W flag.
docommand YWa "${vg_admin} -fyW ${s}" 0 "" IGNORE
docommand --stdout_is_file YWg "${get} -p -r1.1 ${s}" 0 inputs/foo.kw-expansion.W-only.binary IGNORE

# Expand the W and C flags.
docommand YCWa "${vg_admin} -fyW,C ${s}" 0 "" IGNORE
docommand --stdout_is_file YCWg "${get} -p -r1.1 ${s}" 0  inputs/foo.kw-expansion.WC-only.binary IGNORE

remove ${g} ${s} foo

# Now, testing for %A% and %I%
g=quux
s="s.${g}"
z="z.${g}"

remove "${g}" "${s}" "${z}"
copy y-flag-quux-initial inputs/quux.initial.txt "${g}"

docommand YA1 "${admin} -i${g} ${s}" 0 "" IGNORE
remove "${g}"

# Delete the y flag (so all keywords are expanded)
docommand YA2 "${vg_admin} -dy ${s}" 0 "" IGNORE
docommand --stdout_is_file YA3 "${get} -p -r1.1 ${s}" 0 inputs/quux.kw-expansion.default.binary IGNORE

# Disable expansion of %Z% and %I%, and check that it is still expanded in
# %A%.
docommand YA4 "${vg_admin} -fyA,M ${s}" 0 "" IGNORE
docommand --stdout_is_file YA5 "${get} -p -r1.1 ${s}" 0 inputs/quux.kw-expansion.AM.binary IGNORE

# Disable M as well and check again.
# Disable expansion of %Z% and %I%, and check that it is still expanded in
# %A%.
docommand YA6 "${vg_admin} -fyA ${s}" 0 "" IGNORE
docommand --stdout_is_file YA7 "${get} -p -r1.1 ${s}" 0 inputs/quux.kw-expansion.A.binary IGNORE

remove "${s}" "${g}" "${z}" foo command.log last.command core
remove expected.stderr got.stderr expected.stdout got.stdout

success
