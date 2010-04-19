#! /bin/sh
# autoogen.sh: Part of GNU CSSC.
# 
# Copyright (C) 2001,2002,2007 Free Software Foundation, Inc. 
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



get_subdirs()
{
for src_script in configure.ac configure.in
do
  if test -f $src_script 
  then
      grep "^ *AC_CONFIG_SUBDIRS" < "$src_script" | 
      sed -e 's/^ *AC_CONFIG_SUBDIRS//' | 
      tr -d "()" 
      break
  fi
done
}

generate()
{
    skip_ah="true"
    for src_script in configure.ac configure.in
    do
        if test -f $src_script 
        then
        	if grep "^A[CM]_CONFIG_HEADER" < $src_script >/dev/null
        	then
        	    skip_ah=false
        	fi
            break
        fi
    done
    echo aclocal...    && aclocal && 
    if $skip_ah
    then
	echo skipping autoheader...
    else
	echo autoheader... && autoheader
    fi && 
    echo automake...   && automake -a -c && 
    echo autoconf...   && autoconf

    ( generate_in $(get_subdirs) )
}

generate_in () 
{
    for dirname
    do
      echo Generating configure script in "$dirname"
      ( cd "$dirname" && generate ) || break
    done
}

( cd googletest && libtoolize --force )
if generate_in . 
then
    echo "All successful; you can now run configure and make."
else
    echo FAILED.
    false
fi

