#! /bin/sh

shellcheck_postprocess() {
    sed -e 's/: note:/: warning:/'
}

run_shellcheck() {
    # use fd 3 as a place to put  shellcheck's stderr output.
    # send its stderr into a pipe where it can be modified.
    # then reverse, this, so our stdout shows shellcheck's stdout
    # and our stderr shows shellcheck's modified stderr.
    shellcheck "$@" | shellcheck_postprocess
}


awk -e '
{
    dir=$1;
    sub("/[^/]*$", "", dir);
    files[dir] = files[dir] " " $1;
}

END {
    for (dir in files) {
        printf("%s:%s\n", dir, files[dir]);
    }
}
' | {
    IFS=":"
    outer_rv=0
    while read -r dir paths
    do
        (
            IFS=" "
            if cd "${dir}"
	    then
		# We specifically want to word split ${paths} here.
		# shellcheck disable=SC2086
		run_shellcheck -f gcc -x -e SC2121,SC2006,SC2119,SC2012,SC2196,SC2268,SC2003 ${paths} || exit 1
	    fi
        ) || outer_rv=$?
    done
    ( exit "${outer_rv}" )
}
