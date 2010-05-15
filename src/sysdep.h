/*
 * sysdep.h: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 1997,2007 Free Software Foundation, Inc.
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
 *
 * Includes system dependent header files.
 *
 * @(#) CSSC sysdep.h 1.2 93/11/10 04:48:37
 *
 */

#ifndef CSSC__SYSDEP_H__
#define CSSC__SYSDEP_H__

#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>


#ifdef HAVE_PROTOTYPES_H
#include <prototypes.h>
#endif

#ifdef HAVE_IO_H
#include <io.h>
#endif

#include <sys/wait.h>
#include <sys/stat.h>

#ifdef HAVE_PROCESS_H
#include <process.h>
#endif

#ifdef CONFIG_UIDS
#include <pwd.h>
#endif

#endif /* __SYSDEP_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
