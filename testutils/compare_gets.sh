#! /bin/sh

#
# compare_gets
#
# This script compares the results of getting each SID from a list of 
# SCCS files with each of two implementations of sccs-get.  
#
# usage:
#   compare_gets.sh dir1 dir2 file [ file ... ]
# 
# example:
#   compare_gets.sh /usr/ccs/bin /usr/local/libexec/cssc s.foo.c s.bar.c
# 
# If you have a source tree to compare, you could use it like this:-
#   find /usr/src -name s.\* -print | 
#        xargs compare_gets.sh /usr/ccs/bin /usr/local/libexec/cssc

tfile1="/tmp/scmp_tfile1.$$"
tfile2="/tmp/scmp_tfile2.$$"
rv=0

# get_sid_list
# 
# arg1: The name of the SCCS file.
# 
get_sid_list () { 
    # Lists the SIDs in the named file, on stdout.
    "$dir1/prs" -a -l -r1.1 -d:I: "$1" | nl -ba | sort -rn | awk '{print $2;}'
}

# compare_sid_getting 
#
# arg1: The name of the SCCS file.
# 
compare_sid_getting () {
    sfile="$1"
    echo "$sfile:"
    rm -f $tfile1 $tfile2 
    for sid in `get_sid_list "$sfile"`
    do
	echo "Comparing $sfile at SID $sid..."

	"$get1" -s -p -r$sid "$sfile" > "$tfile1" 2>/dev/null 
	s1=$?
	if test $s1 -gt 1
	then
	    echo "Fatal error in $get1." >&2
	    rm -f  $tfile1 $tfile2
	    exit $s1
	fi

	
	"$get2" -s -p -r$sid "$sfile" > "$tfile2" 2>/dev/null 
	s2=$?
	if test $s2 -gt 1
	then
	    echo "Fatal error in $get2." >&2
	    rm -f  $tfile1 $tfile2
	    exit $s2
	fi

	diff "$tfile1" "$tfile2" >/dev/null
	s3=$?
	rm -f  $tfile1 $tfile2
	if test $s3 -gt 1
	then
	    echo "Fatal error in diff." >&2
	    exit $s3
	fi
	
	if test $s3 -ne 0
	then
	    echo "File $sfile differs between $get1 and $get2 at SID $sid" >&2
	    rv=1
            return 1
        fi
    done
    return 0
}


dir1="$1"
dir2="$2"
get1="$dir1/get"
get2="$dir2/get"

shift
shift

if test -x "$get1"
then
    true
else
    echo "$get1 is not exexutable." >&2
    exit 2
fi

if test -x "$get2"
then
    true
else
    echo "$get2 is not exexutable." >&2
    exit 2
fi

rv=0

for filename
do
    echo SIDs in file $filename...
    compare_sid_getting "$filename" || break
done
exit $rv


# The remaining arguments should be a list of files.

