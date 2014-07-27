. ../common/test-common
. ../common/real-thing

# This test creates an empty executable file, checks it in with admin, and then
# verifies that the resulting s-file is also executable.
g=foo
s=s.$g

cleanup() {
    cmd="rm -f ${g} ${s}"
    echo "${cmd}" >> command.log
    $cmd
}
cleanup

expect_args() {
    function_name="$1"
    expected="$2"
    got="$3"

    detail="expected ${expected} arguments, but got ${got}"
    if test "${got}" -lt "${expected}"; then
	miscarry "Too few arguments to ${function_name}: ${detail}"
    elif test "${got}" -gt "${expected}"; then
	miscarry "Too many arguments to ${function_name}: ${detail}"
    fi
}

setup() {
    expect_args setup 3 $#
    #label="$1"
    #mask="$2"
    #mode="$3"
    cleanup
    umask "${2}"
    docommand --silent "${1}-setup0" "touch ${g}" IGNORE IGNORE IGNORE || miscarry "failed to create ${g}" &&
    docommand --silent  "${1}-setup1" "chmod ${3} ${g}" 0 "" ""
    docommand "${1}"        "${admin} -i${g} -n ${s}" 0 IGNORE IGNORE
}


should_support_execute_bits() {
    expect_args should_support_execute_bits 0 $#
    if "${TESTING_CSSC}"; then
	true
    else
	case "`uname -s`" in
	    SunOS) true;;
	    *) false;;
	esac
    fi
}

if should_support_execute_bits; then
    # Not using ! is a workaround for the fact that Solaris /bin/sh doesn't support it.
    true
else
    echo "Your version of SCCS is not expected to support execute permissions; skipping."
    exit 0
fi

file_permissions() {
    expect_args file_permissions 1 $#
    ls -ld "$1" 2> /dev/null | sed -n -e '1 s/^.\(.........\).*/\1/p' 
}

is_owner_executable() {
    expect_args is_owner_executable 1 $#
    case `file_permissions "$1"` in
	??x*) true;;
	*) false;;
    esac
}
  
is_group_executable() {
    expect_args is_group_executable 1 $#
    case `file_permissions "$1"` in
	?????x*) true;;
	*) false;;
    esac
}

is_other_executable() {
    expect_args is_other_executable 1 $#
    case `file_permissions "$1"` in
	????????x*) true;;
	*) false;;
    esac
}

execute_perms() {
    expect_args execute_perms 1 $#
    result=""
    if is_owner_executable "$1"; then
	result="${result}u"
    fi
    if is_group_executable "$1"; then
	result="${result}g"
    fi
    if is_other_executable "$1"; then
	result="${result}o"
    fi
    echo "${result}"
}

selfcheck() {
    expect_args selfcheck 2 $#
    chmod_mode="$1"
    execute_perms_expected="$2"
    shift 2
    touch "${g}" || miscarry "self-check: cannot create ${g}"
    chmod "${chmod_mode}" "${g}" || miscarry "self-check: cannot chmod ${chmod_mode} ${g}"
    perms="`execute_perms ${g}`"
    if test "${perms}" != "${execute_perms_expected}"; then
	fail "self-check: ${g} should be have execute permissions for ${execute_perms_expected} but actually has them for ${perms:-nobody}"
    fi
}


### Some self-checks
selfcheck 0160 u
selfcheck 0616  g
selfcheck 0441   o
selfcheck 0170 ug
selfcheck 0171 ugo
selfcheck 0071  go

    
(
    setup x01 0077 0700
    test -f "${g}" || miscarry "where is ${g}?"
    if is_owner_executable "$g"; then
        # Not using ! is a workaround for the fact that Solaris /bin/sh doesn't support it.
	true
    else
        miscarry "Cannot create an executable file"
    fi
    
    echo_nonl "${test_script}:x02..."
    if test "`execute_perms ${s}`" = u; then
        echo passed
    else
        fail "x02: ${s} should be owner-executable"
    fi
) || rv=1

(
    setup x03 0007 0770
    echo_nonl "${test_script}:x04..."
    if test "`execute_perms ${s}`" = ug; then
        echo passed
    else
        fail "x04: ${s} should be owner- and group-executable"
    fi
) || rv=1

(
    setup x05 0000 0777
    echo_nonl "${test_script}:x06..."
    if test "`execute_perms ${s}`" = ugo; then
        echo passed
    else
        fail "x06: ${s} should be owner-, group- and world-executable"
    fi
) || rv=1


# The following tests verify that we DO set execute bits on the
# history file which were not set on the original file (from which the
# history file was created).  Solaris does this.
#
# The result of this is that when get is used on the resulting history
# file, that file could be world-executable even if the original file
# (offered via "admin -i") was not.  It might seem logical to avoid
# following this example (since the execute bits on the original file
# may have been set that way to avoid world-execute permissions).
# However, the Principle of Least Astonishment guides us to treat the
# execute bits in the same way as the read bits are treated (all
# versions of SCCS create world-readable gotten files if the umask
# allows it).
#
# These tests check the behaviours described above.
(
    setup x07 0000 0700

    echo_nonl "${test_script}:x08..."
    perms="`execute_perms ${s}`"
    case "${perms}" in
	ugo) echo passed;;
	u|ug|ugo) fail "x08: ${s} should be mode 0777 when umask is 0 and the -i file is executable.";;
	"") fail "x08: execute permissions not copied to history file at all";;
	*) miscarry "unexpected execute perms ${perms} for file ${s}";;
    esac || exit 1
) || rv=1

# On SCO OpenServer and CSSC, the 'x' flag, if set, makes the g-file executable.
setup x09 0177 0600
# The gfile exists but is not executable.
docommand x10 "test -x ${g}" 1 "" ""

if docommand x11 "${admin} -fx ${s}" IGNORE IGNORE IGNORE; then
    # x-flag is suported. The s-file should still not be executable, since the x flag
    # controls the mode of the gotten file, not the history file.
    docommand x12 "test -x ${s}" 1 "" ""
    docommand x13 "rm -f ${g}" 0 "" ""
    docommand x14 "${get} ${s}" 0 IGNORE IGNORE
    # The gotten file should be executable, now (i.e. controlled by the x flag)
    docommand x15 "test -x ${g}" 0 "" ""
    # The history file should still not be executable
    docommand x16 "test -x ${s}" 1 "" ""
    # Unset the x-flag
    docommand x17 "${admin} -dx ${s}" IGNORE IGNORE IGNORE
    # The history file should still not be executable
    docommand x18 "test -x ${s}" 1 "" ""
    # Get the file again, verify that it exists but is not executable.
    docommand x19 "${get} ${s}" 0 IGNORE IGNORE
    docommand x20 "test -f ${g}" 0 "" ""
    docommand x21 "test -x ${g}" 1 "" ""
else
    # Cannot do the rest of the x-flag tests.
    echo "Your version of SCCS does not support the 'x' flag; skipping some tests."
fi

# Verify that we correctly set the executable bits on the history file
# when there is no argument to -i (and hence we are reading the
# initial body from stdin).
cleanup
umask 077
docommand x22 "touch ${g}" IGNORE IGNORE IGNORE || miscarry "failed to create ${g}" &&
docommand x23 "chmod 0500 ${g}" 0 "" ""
# We present the initial body via stdin.   It's executable.
docommand x24 "${admin} -i -n ${s} < ${g}" 0 IGNORE IGNORE
docommand x25 "test -x ${s}" 0 "" ""


cleanup
exit $rv
