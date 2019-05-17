/*
 * cssc.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 2001, 2002, 2007, 2008, 2009, 2010, 2011,
 *  2014, 2019 Free Software Foundation, Inc.
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
 * cssc.h: Master include file for CSSC.
 *
 */
#ifndef CSSC__CSSC_H__
#define CSSC__CSSC_H__

// Get the definitions deduced by "configure".
#include <config.h>

#include <string>

/* Define if you want to open SCCS files in binary instead of text mode.
 * If you do this, you will probably need to jump through hoops on
 * Microsoft systems, in order to avoid falling over all those
 * carriage returns.
 */
#undef  CONFIG_OPEN_SCCS_FILES_IN_BINARY_MODE

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//           Tunable
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
#define CONFIG_FILE_NAME_GUESSING

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//           Deduced
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

#if defined HAVE_GETEUID && defined HAVE_GETEGID
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
#if defined __CYGWIN__
#define CONFIG_CAN_HARD_LINK_AN_OPEN_FILE 0
#else
#define CONFIG_CAN_HARD_LINK_AN_OPEN_FILE 1
#endif

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//           MS-DOS
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
#undef  CONFIG_USE_ARCHIVE_BIT

#undef  CONFIG_MSDOS_FILES
#define CONFIG_DJGPP

std::string prompt_user(const char *prompt);
unsigned long cap5(unsigned long); // see cap.cc
bool is_id_keyword_letter(char ch);

/* functions from environment.cc. */
bool binary_file_creation_allowed (void);
long max_sfile_line_len(void);
void check_env_vars(void);

#endif

/* Local variables: */
/* mode: c++ */
/* End: */
