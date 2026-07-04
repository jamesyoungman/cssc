#! /bin/sh

# This defines TESTING_CSSC to "true" if we are testing CSSC, or defines 
# it to false if we are not.  In the latter case we are usually testing 
# some vendor's implementation.

if test -z "${admin}"; then
    echo '${admin} is not set, please source common/test-common before common/real-thing' >&2
    exit 1
fi

if ( ${admin} -V 2>&1 ; echo umsp )  | grep CSSC >/dev/null
then
    TESTING_CSSC=true
else
    TESTING_CSSC=false
fi
