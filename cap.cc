/*
 * cap.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,2000 Free Software Foundation, Inc. 
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 * Utilities for limiting output to the constraints of the file format.
 */

#include "cssc.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: cap.cc,v 1.2 2000/11/26 20:51:46 james_youngman Exp $";
#endif

template<class T> const T& cap_min(const T& a, const T& b)
{
  return (a < b) ? a : b;
}

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
