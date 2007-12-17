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

#ifdef HAVE_FCNTL_H 
#include <fcntl.h>
#endif

#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_PROTOTYPES_H
#include <prototypes.h>
#endif

#ifdef HAVE_IO_H
#include <io.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#ifdef CONFIG_WAIT_IS_A_USELESS_MACRO
#undef wait
#endif
#endif

#ifdef HAVE_PROCESS_H
#include <process.h>
#endif

#ifdef CONFIG_MSDOS_FILES
#if defined(__BORLANDC__) && defined(__STDC__)
#define far __far
#endif
#include <dos.h> 
#else
#include <sys/stat.h>
#endif

#ifdef CONFIG_UIDS
#include <pwd.h>
#endif

#ifdef CONFIG_SHARE_LOCKING
#include <share.h>
#endif

#ifdef CONFIG_DECLARE_FDOPEN
extern "C" FILE * CDECL fdopen(int, const char *);
#endif

#endif /* __SYSDEP_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
