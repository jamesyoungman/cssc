#! /bin/sh
# docommand.sh:  Self-tests for the test infrastructure

# Import common functions & definitions.
. ../common/test-common


docommand t1 "true" 0 "" ""
docommand t2 "false" 1 "" ""

echo_nonl "initial/docommand.sh:t3..."
if ( docommand t3 "false" 0 "" "" ) >/dev/null 2>&1 ; then
    echo "docommand failed to detect a nonzero exit status" >&2
    exit 1
else
    echo "passed"
fi

docommand s1 "echo hello" 0 "hello\n" ""
echo_nonl "initial/docommand.sh:s2..."
if ( docommand s2 "echo hello" 0 "not-hello" "" ) >/dev/null 2>&1 ; then
    echo "docommand failed to detect a mismatched stdout" >&2
    exit 1
else
    echo "passed"
fi

docommand e1 "echo hello >&2" 0 "" "hello\n"
echo_nonl "initial/docommand.sh:e2..."
if ( docommand e2 "echo hello >&2" 0 "" "not-hello" ) >/dev/null 2>&1 ; then
    echo "docommand failed to detect a mismatched stderr" >&2
    exit 1
else
    echo "passed"
fi

echo this-is-the-stdin-input | docommand p1 "cat" 0 "this-is-the-stdin-input\n" "" 
echo_nonl "initial/docommand.sh:p2..."
if echo this-is-not-the-stdin-input | ( docommand p2 "cat" 0 "this-is-the-stdin-input\n" "" ) >/dev/null 2>&1; then
    echo "docommand failed to detect a mismatched stdout when stdin was a pipe " >&2
    exit 1
else
    echo passed
fi
    

success
