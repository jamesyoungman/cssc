/* 
 * cssc.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998,2001 Free Software Foundation, Inc. 
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 * cssc.h: Master include file for CSSC.
 *
 * $Id: cssc.h,v 1.21 2001/09/29 19:39:41 james_youngman Exp $
 *
 */


#ifndef CSSC__CSSC_H__
#define CSSC__CSSC_H__

#undef TESTING

// Get the definitions deduced by "configure".
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif







#undef  CONFIG_DECLARE_STAT
#undef  CONFIG_DECLARE_GETPWUID
#undef  CONFIG_DECLARE_TIMEZONE
#undef  CONFIG_DECLARE_TZSET
#undef  CONFIG_DECLARE_FDOPEN




//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//           TODO
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

/* Ancient versions of SLS Linux apparently need this. */
#define CONFIG_WAIT_IS_A_USELESS_MACRO

/* Switches we want to pass to diff.  Perhaps we might want to */
/* pass special switches to GNU diff (e.g. heuristic mode).  Later maybe. */
#undef  CONFIG_DIFF_SWITCHES

/* Define if you want to open SCCS files in binary instead of text mode.
 * If you do this, you will probably need to jump through hoops on
 * Microsoft systems, in order to avoid falling over all those
 * carriage returns.
 */
#undef  CONFIG_OPEN_SCCS_FILES_IN_BINARY_MODE



//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//           Tunable
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
#define CONFIG_LIST_CHUNK_SIZE          16
#define CONFIG_FILE_NAME_GUESSING



//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//           Deduced
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

#if defined(HAVE_UNISTD_H) && defined(HAVE_GETEUID) && defined(HAVE_GETEGID)
#define CONFIG_UIDS
#endif

/*************************************************************/
/*           CYGWIN Support                                  */
/*************************************************************/
/*
 * CYGWIN is the Unix environment for Windows, from Cygnus.
 * It provides a very Unix-like environment under Windows NT;
 * so much so that the configure script is unable to tell the difference.
 */
#if defined(__CYGWIN__)
#define CONFIG_CAN_HARD_LINK_AN_OPEN_FILE 0
#else
#define CONFIG_CAN_HARD_LINK_AN_OPEN_FILE 1
#endif




//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//           MS-DOS
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
#undef  CONFIG_USE_ARCHIVE_BIT
  
/* I'm afraid that if you want to change CSSC's idea of what goes at
 * the end of the line then this macro will not help very much.
 * Ninety percent of the cases where detecting the end-of-line is
 * useful are just dealt with tith a literal '\n'.   I'm not sure if
 * it is useful to open the file in text mode (given the nonprintable
 * control character \001) but that seems the best plan to me.  Please
 * let me know how it goes.   Patches, as always, gleefully welcomed!
 * See docs/patches.txt for further information.
 */
#ifndef CONFIG_EOL_CHARACTER
#define CONFIG_EOL_CHARACTER ('\n')
#endif


#undef  CONFIG_MSDOS_FILES
#undef  CONFIG_BORLANDC
#define CONFIG_DJGPP


// Include macros for deciding if a <stdio.h> routine has returned an
// error status.
#include "ioerr.h"


#include "defaults.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "err_no.h"

#ifndef NO_COMMON_HEADERS
#include "quit.h"
#include "mystring.h"
#include "file.h"


// Some declarations are only useful if we know what a "mystring" is.
//
void
split_filename(const mystring &fullname,
               mystring& dirname, mystring& basename);
mystring prompt_user(const char *prompt);

#endif /* NO_COMMON_HEADERS */

typedef unsigned short seq_no;

unsigned long cap5(unsigned long);
bool is_id_keyword_letter(char ch);

/* functions from environment.cc. */
bool binary_file_creation_allowed (void);
long max_sfile_line_len(void);
void check_env_vars(void);

#endif

/* Local variables: */
/* mode: c++ */
/* End: */
