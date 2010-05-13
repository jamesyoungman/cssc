# Makefile.am: Part of GNU CSSC.
#
# Copyright (C) 1997,1998,1999,2000,2001,
#		2002,2003,2004 Free Software Foundation, Inc.
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
# 
#
## Process this file with automake to generate "Makefile.in"
##
## Since it doesn't start with "##", the following line will be copied 
## into Makefile.in, and become the first line.
# Running ./configure will generate a Makefile from this file.


# We keep "tests" as the last named subdirectory in SUBDIRS,
# so that when we do "make check", the "Tests passed" message
# is emitted last (we wouldn't want "entering directory unused"
# messages to make that scroll off the screen).
# 
# We run the unit tests before the regression tests in order to make
# it more obvious where the bug is when there is a test failure.
SUBDIRS = gl bsd docs testutils auxfiles src sccs-cgi googletest unit-tests tests 

EXTRA_DIST = ChangeLog ChangeLog.1
