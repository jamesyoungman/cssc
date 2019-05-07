/*
 * defaults.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 2001, 2007, 2008, 2009, 2010, 2011, 2014,
 *  2019 Free Software Foundation, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 *
 * Sets the default values of configuration macros left undefined.
 *
 * @(#) CSSC defaults.h 1.1 93/11/09 17:17:46
 *
 */

#ifndef CSSC__DEFAULTS_H__
#define CSSC__DEFAULTS_H__

#ifndef LIDENT
#define LIDENT(ident) cssc_##ident
#endif

#define NORETURN void

#ifndef POSTDECL_NORETURN
#ifdef __GNUC__
/* GNU C */
#define POSTDECL_NORETURN __attribute__ ((noreturn))
#else
/* Not GNU C */
#define POSTDECL_NORETURN /* does not return */
#endif
#endif


#ifndef CDECL
#ifdef __BORLANDC__
#define CDECL __cdecl
#else
#define CDECL
#endif
#endif /* CDECL */

#ifndef CONFIG_NULL_FILENAME
#define CONFIG_NULL_FILENAME "/dev/null"
#endif /* CONFIG_NULL_FILENAME */


#endif /* __DEFAULTS_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
