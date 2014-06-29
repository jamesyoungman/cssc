#! /bin/sh

deleteprefix() {
    sed -n -e 's_^unit-tests/__p' 
}

findproblems() {
	sourcedir="$1"
	tarfile="$2"
	shift 2

	set -Ceu
	tar ztf ${tarfile} | cut -d/ -f2-  | deleteprefix |sort >| "${released}"

	( cd "${sourcedir}" && find unit-tests/googletest  \
	    \(  -name .svn -o -name .gitignore  \) -prune , -type f ) | deleteprefix | sort >| "${ondisk}"

	comm -13 "${released}" "${ondisk}" >| "${problems}"
       
	if [[ -s "${problems}" ]] ; then
	    exec >&2
	    echo "Problem: some files in googletest were not included in the release."
	    cat "${problems}"
	    exit 1
	else
	    echo "${tarfile} seems to include all the googletest files, good."
	fi
}

main() {
	set -Ceu
	released=$(mktemp)
	ondisk=$(mktemp)
	problems=$(mktemp)

	findproblems "$@"
	rv=$?

	rm -f "${released}" "${ondisk}" "${problems}"
	exit $rv
}


main "$@"

