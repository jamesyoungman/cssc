# Hey, Emacs!  This is a -*- sh -*- script.
#
# The test suite fails if run by root, because when you are root,
# "test -w foo" returns 0 even for read-only files because root can
# write to them.
#
# The test suite depends on being able to accurately detect a readonly file.

# Check that abandon_test_script actually works.
if ( abandon_test_script 2>/dev/null; true )
then
    echo 'Please source test-common before not-root.' >&2
    exit 1
fi

# Execute in a subshell so that we can use "trap ... 0" and
# ensure that the temporary file is removed before we reach
# the end of this file.

(
f=/tmp/foo.$$.tmp
# Use rm and echo rather than risking a missing "touch".
rm -f $f ; echo > $f

# Remove temporary file on exit.
trap "rm -f $f" 0


chmod 400 $f
if test -f $f
then
	if test -w $f
	then
		abandon_test_script "Please do not run the suite as root"
	fi
else
	abandon_test_script "Could not create $f (or buggy test(1))"
fi
unset f
) || exit $?
