. ../common/test-common
. ../common/real-thing

g=foo
s=s.$g
p=p.$g
z=z.$g

cleanup() {
    cmd="rm -f ${g} ${s} ${p} ${z}"
    echo "${cmd}" >> command.log
    $cmd
}
cleanup

# We use -i/dev/null below in order not to spuriously make the test
# suite fail on Dynix.
docommand          x00 "${admin} -i/dev/null -n ${s}" 0 IGNORE IGNORE
# Get the file with the history file not executable; check that the
# gotten file is not executable.
docommand          x01 "${get} ${s}" 0 IGNORE IGNORE
# Make sure the gfile exists at all first.
docommand          x02 "test -f ${g}" 0 "" ""
docommand          x03 "test -x ${g}" 1 "" ""

# Make the history file executable
docommand          x04 "chmod +x ${s}" 0 "" ""
docommand --silent x04-sanity "test -x ${s}" 0 "" ""

# Check that the gotten file is now executable.
rm -f "${g}"
docommand          x05 "${get} ${s}" 0 IGNORE IGNORE
# Make sure the gfile exists at all first.
docommand          x06 "test -f ${g}" 0 "" ""
docommand          x07 "test -x ${g}" 0 "" ""

# get -e should also result in an executable gotten file.
rm -f "${g}"
docommand          x08 "${get} -e ${s}" 0 IGNORE IGNORE
# Make sure the gfile exists at all first.
docommand          x09 "test -f ${g}" 0 "" ""
docommand          x10 "test -x ${g}" 0 "" ""

cleanup
