#! /bin/sh
if ( 
    set -x 
    aclocal && autoheader && automake -a -c && autoconf 
)
then
    echo "All successful; you can now run configure and make."
else
    echo FAILED.
    false
fi

