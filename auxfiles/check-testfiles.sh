#! /bin/sh
# check-testfiles.sh -- Check we distributed all the test files we need
# Copyright (C) 2007, 2010, 2019, 2024 Free Software Foundation, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

rv=0


# $1: a directory (source or dest) to make a list of files for.
# $2: a single filename suffix to check
makelist () {
    ls "${1}"/*/*"${2}" | sed -e 's/.*\///' | sort
}

# $1: a single filename suffix to check
diagnose () {
    makelist "${distdir}" "$1" > dist"${1}".txt  &&
    makelist "${srcdir}"  "$1" >  src"${1}".txt  &&
    diff  src"${1}".txt dist"${1}".txt
    rm -f src"${1}".txt dist"${1}".txt
    echo
}


check_shipfiles () {
        distcount=`ls "${distdir}"/*/*"${suffix}" | wc -l`
        srccount=`ls "${srcdir}"/*/*"${suffix}" | wc -l`
        if test "$distcount" -eq "$srccount" ; then
            echo "All $srccount of the $suffix files are accounted for"
        else
            echo "ERROR: Missing $suffix files: source $srccount distributed $distcount" >&2
            rv=1
            diagnose "${suffix}"
        fi
}


main () {
    distdir="$1"
    srcdir="$2"
    shift 2
    if test "$#" -gt 0 ; then
        for suffix ; do
                check_shipfiles "$suffix"
        done
        exit $rv
    else
        echo "You did not specify any test file suffixes." >&2
        exit 1
    fi
}

main "$@"
