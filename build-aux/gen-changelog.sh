#! /bin/sh
# Generate the ChangeLog for CSSC.

# Copyright (C) 2015, 2016, 2017, 2018, 2019, 2024 Free Software
# Foundation, Inc.
# Written by James Youngman.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

set -u
top_srcdir="$1"

"${top_srcdir}/build-aux/gitlog-to-changelog" \
    --srcdir="${top_srcdir}" \
    --ignore-matching='IGNORE_THIS' \
    --since='2019-05-06' \
    --strip-cherry-pick \
    --amend="${top_srcdir}/build-aux/git-log-fix" \
  && cat "${top_srcdir}/ChangeLog.1" "${top_srcdir}/ChangeLog.2" "${top_srcdir}/ChangeLog.3"
