#! /bin/sh
# writable.sh:  Will get over-write a writable file?

# Import common functions & definitions.
. test-common

f=wrtest
remove s.$f
: > $f
$admin -n -i $f s.$f >/dev/null 2>&1 || fail admin failed.
test -r s.$f         || fail admin did not create s.$f
remove $f

echo foo > $f
chmod +w $f

# The get should fail.
$get s.$f >/dev/null 2>&1 && fail get returned success when gfile was writable
gfile=_g.$f
remove _g.$f
test -e $gfile	    && miscarry could not remove _g.$f
eval "$get -G$gfile s.$f" >/dev/null 2>&1|| fail Could not get s.$f into $gfile.
remove $f s.$f $gfile

success

