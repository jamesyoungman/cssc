#! /bin/sh
#
# show-disp.sh: CVS utility to break down CVS-controlled files
#               by their status with respect to the repository.
#
#    This file is part of GNU CSSC.
#
#    Copyright (C) 1997, 2000 Free Software Foundation, Inc. 
# 
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
# 
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
# 
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA.
#
# $Id: show-disp.sh,v 1.8 2001/09/29 19:39:41 james_youngman Exp $
# 

cvs -z3 status "$@" 2>&1 </dev/null |
 sed \
	-e 's/^File: no file/File:/'  | 
 sed -n -e 's/^File: \([^ 	]*\).*Status: \(.*\)$/\1:\2/p' \
	-e '/^cvs/p' | awk '

BEGIN { FS=":"; dir=""; }

## Take note when we change directory.
#
/^cvs (status|server): Examining/ { 
	dir = $0;
	gsub("cvs status: Examining ", "", dir);
	gsub("cvs server: Examining ", "", dir);

	if (dir == ".") {
		dir = "";
	} else {
		dir = dir "/"; 
	}

	# We have now fully processed this line.
	# Do not let any of the other rules process
	# it.
	next;
}

## Print any status messages.
#
/^cvs status: / { print; next; }

## All other lines are Filename:Disposition
#
!/^cvs status:/ {
	file_list[$2] = file_list[$2] " " dir $1; 
	++count[$2]; 
} 

## Output a string, without going over 80 columns.
#
function output(str, startcol)
{
  column = startcol;
  split(str, names, " ");
  for (i in names)
    {
      len = length(names[i]);
      if (column + len > 78) {
	# Begin printing at column startcol on the new line.
	printf("\n%*s", startcol, "");
	column = startcol;
      }
      printf("%s ", names[i]);
      column += ( len + 1);
    }
  printf("\n");
}

## At the end of procressing, indicate the dispositions of 
## files other than those that are Up-to-date.
#
END {
  n_up2date = count["Up-to-date"];
  if (n_up2date > 0) { 
  	printf("%d files Up-to-date.\n", n_up2date);
  }

  for (disposition in file_list)
    {
      if (disposition != "Up-to-date")
	{
	  prefix = sprintf("%s: %d: ",
			disposition,
			count[disposition]);
	  printf("%s", prefix);
	  output(file_list[disposition], length(prefix));
	}
    }
}'
