#! /bin/sh
# writable.sh:  Will get over-write a writable file?

# Import common functions & definitions.
. test-common

remove command.log log log.stdout log.stderr

f=wrtest
gfile=_g.$f
remove s.$f

# Generate empty file.
: > $f

# Create the s. file and make sure it exists.
docommand W1 "$admin -n -i$f s.$f" 0 "" IGNORE

test -r s.$f         || fail admin did not create s.$f
remove $f
echo foo > $f
chmod +w $f

# Try running get when gfile was writable -- it should fail.
docommand W2 "$get s.$f" 1 IGNORE IGNORE
remove $gfile
test -e $gfile	    && miscarry could not remove _g.$f

# Now run get with the -G option and it should work even
# though the file's usual name is occupied by a writable file.
docommand W3 "$get -G$gfile s.$f" 0 "1.1\n0 lines\n" IGNORE
remove $f s.$f $gfile

success

