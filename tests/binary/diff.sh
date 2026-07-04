#! /bin/sh
# diff.sh:  Testing for the system diff utility.

# Import common functions & definitions.
. ../common/test-common.sh
. ../common/real-thing.sh
. ../common/config-data.sh

if $binary_support
then
    true
else
    echo "Skipping these tests -- no binary file support."
    exit 0
fi


set_and_maybe_print_step_label_with_dots t1
remove test/d1 test/d2 test
mkdir test
echo hello > test/d1
echo hello > test/d2
${DIFF} test/d1 test/d2 >got.stdout 2>got.stderr
rv=$?
if test $rv -ne 0
then
    fail "${DIFF} returns nozero for identical files"
fi
echo passed




set_and_maybe_print_step_label_with_dots t2
remove test/d2
echo world > test/d2
${DIFF} test/d1 test/d2 >got.stdout 2>got.stderr
rv=$?
if test $rv -eq 0
then
    fail "${DIFF} returns zero for nonidentical files"
fi
remove test/d1 test/d2 test
echo passed

success
