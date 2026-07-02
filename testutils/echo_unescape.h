/*
 * echo_unescape.h: Part of GNU CSSC.
 *
 *  Copyright (C) 2026 Free Software Foundation, Inc.
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
 */
#ifndef INC_CSSC_TESTUTILS_ECHO_UNESCAPE_H
#define INC_CSSC_TESTUTILS_ECHO_UNESCAPE_H 1

#include <stdbool.h>		// bool
#include <stdlib.h>		// size_t

/* Deocde backslash escape sequences in INPUT, writing the result to
   OUTPUT (without terminating null character).  OUTPUT must be at
   least as large as INPUT (and may be the same buffer).  If we find a
   \c escape, set *INHIBIT_NEWLINE.
*/
size_t echo_unescape(const char *input,
		     char *output,
		     bool *inhibit_newline);

#endif
