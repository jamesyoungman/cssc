#! /bin/sh
# Checks out a copy of the Google C++ Testing Framework.
url='http://googletest.googlecode.com/svn/trunk/'
module='googletest'

usage() {
    echo "usage: $0 svn-version"
}

main() {
    if [ -z "$module" ]; then
	echo "Please specify which directory to put the googletest code in (by editing $0)" >&2
	usage >&2
	exit 1
    fi
    
    if [ -d "$module" ] && [ -d "$module"/.svn ]; then
	( cd "$module" && svn update -r "$1" ) || return 1
    else
	svn --non-interactive checkout "$url"@"$1" "$module" || return 1
    fi
}

main "$@"
