/*
 * cap.cc: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 1997,2000,2007, 2008, 2009, 2010, 2011, 2014  Free Software Foundation, Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 * Utilities for limiting output to the constraints of the file format.
 */

#include <config.h>

#include "cssc.h"

namespace {

template<class T> constexpr T cap_min(T a, T b)
{
  return (a < b) ? a : b;
}

}  // unnamed namespace

// Many data fields in SCCS files are limited to five digits; hence
// some output fields are capped at 99,999.
//
// An example of such a field is the delta summary line; the fields
// indicating the numbers of lines added/removed/unchanged are capped
// at five digits.
//
unsigned long
cap5(unsigned long n)
{
  return cap_min(n, 99999uL);
}

/* Local variables: */
/* mode: c++ */
/* End: */
