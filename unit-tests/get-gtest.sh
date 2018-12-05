#! /bin/sh
# This should match git submodule configuration
module=../googletest

usage() {
    echo "usage: $0 svn-version"
}

main() {
    if [ -z "$module" ]; then
	echo "Please specify which directory to put the googletest code in (by editing $0)" >&2
	usage >&2
	exit 1
    fi
    if [ -z "$1" ]; then
	echo "Please specify a commit or tag from $module" >&2
	usage >&2
	exit 1
    fi

    if [ ! -d "$module" ] || [ ! -e "$module"/.git ] || \
	   [ ! -d "$module"/googletest ]; then
	echo "error: $module is not a googletest git repo" >&2
	exit 1
    fi
    head=$(git -C "$module" log -n 1 --pretty=%H HEAD)
    [ -n "$head" ] || {
	echo "error getting hash of HEAD in $module" >&2
	exit 1
    }
    trgt=$(git -C "$module" log -n 1 --pretty=%H "$1")
    [ -n "$trgt" ] || {
	echo "error: is $1 is $1 a valid tag or commit ID in $module?" >&2
	exit 1
    }
    if [ "$head" != "$trgt" ]; then
	git -C "$module" checkout $1 || {
	    echo "error checking out $1 in $module" >&2
	    usage >&2
	    exit 1
	}
	git add "$module"
	git commit -m "Checked out googletest $1"
    fi
    ln -sf "$module"/googletest .
}

git submodule update
main "$@"
