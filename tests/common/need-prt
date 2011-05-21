#! /bin/sh

if test -z "${prt}"; then
    echo '${prt} is not set, please source common/test-common before common/need-prt' >&2
    exit 1
fi

# Find out if we have a "prt" executable. 

if test -x "${prt}"
then
    HAVE_PRT=true
else
    "${prt}" ; rv=$?
    if test $rv -gt 120 
    then
	HAVE_PRT=false
    else
	# We passed no arguments to prt so it returned an error code
	# but that error code is smaller than the error code we would have 
	# got if the executable had been missing. 
	HAVE_PRT=true
    fi
fi

$HAVE_PRT || {
    echo "The tests in $0 will not be carried out on this system because ${prt} is missing."

    # Exit successfully because we want the test suite to pass when run against
    # SCCS implementations that lack prt.
    exit 0
}
