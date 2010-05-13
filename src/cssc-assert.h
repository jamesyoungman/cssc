/*
 * cssc-assert.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1999,2001,2007 Free Software Foundation, Inc. 
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
 */
#ifndef INC_CSSC_ASSERT_H
#define INC_CSSC_ASSERT_H 1

#include "defaults.h"


NORETURN assert_failed(const char *file, int line, const char *func,
                       const char *test) POSTDECL_NORETURN;

#ifdef __GNUC__
#define ASSERT(test) ((test) ? (void) 0                                 \
                             : assert_failed(__FILE__, __LINE__,        \
                                             __PRETTY_FUNCTION__, #test))
#else
#define ASSERT(test) ((test) ? (void) 0                                 \
                             : assert_failed(__FILE__, __LINE__, "", #test))
#endif

#endif
