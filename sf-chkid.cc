/*
 * sf-chkid.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997, Free Software Foundation, Inc. 
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
 * 
 */

#include "cssc.h"
#include "sccsfile.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-chkid.cc,v 1.5 1998/01/24 14:09:06 james Exp $";
#endif

bool
is_id_keyword_letter(char ch)
{
  return strchr("MIRLBSDHTEGUYFPQCZWA", ch) != NULL;
}


/* Returns true if the string contains a valid SCCS id keyword. */

int
sccs_file::check_id_keywords(const char *s) {
	s = strchr(s, '%');
	while(s != NULL) {
		if (s[1] != '\0' 
		    && is_id_keyword_letter(s[1])
		    && s[2] == '%') {
			return 1;
		}
		s = strchr(s + 1, '%');
	}
	return 0;
}

/* Local variables: */
/* mode: c++ */
/* End: */
