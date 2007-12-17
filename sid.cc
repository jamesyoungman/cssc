/*
 * sid.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1999,2007 Free Software Foundation, Inc. 
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
 * Members of the classes sid and release.
 *
 */

#include "cssc.h"
#include "sid.h"

#include <ctype.h>

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sid.cc,v 1.19 2007/12/17 21:59:52 jay Exp $";
#endif


mystring sid::as_string() const
{
  char buf[32];
  if (branch || sequence)
    sprintf(buf, "%d.%d.%d.%d", rel, level, branch, sequence);
  else
    sprintf(buf, "%d.%d", rel, level);
  
  return mystring(buf);
}


/* This pointer is used by the template range_list to denote an
   invalid range. */
#if 0
void *invalid_range = new char;
#endif

static int
get_comp(const char *&s) {
	int n = 0;
	char c = *s;
	while (c != '\0') {
		if (c == '.') {
			if (n == 0) {
				return -1;
			}
			s++;
			return n;
		}	
		if (isdigit(c)) {
			n = n * 10 + (c - '0');
		} else {
			return -1;
		}
		c = *++s;
	}
	return n;
}

relvbr::relvbr(const char *s) {
	if (s == NULL) {
		rel = level = branch = 0;
		return;
	}

	rel = get_comp(s);
	level = get_comp(s);
	branch = get_comp(s);

	if (*s != '\0' || rel == 0) {
		rel = level = branch = -1;
	}
}

release::release(const char *s) {
	if (s == NULL) {
		rel = 0;
		return;
	}

	rel = get_comp(s);

	if (*s != '\0' || rel == 0) {
		rel = -1;
	}
}

sid::sid(const char *s) {
	if (s == NULL) {
		sequence = branch = level = rel = 0;
		return;
	}

	rel = get_comp(s);
	level = get_comp(s);
	branch = get_comp(s);
	sequence = get_comp(s);

	if (*s != '\0' || rel == 0 || sequence == -1) {
		rel = -1;
	}
}

int
sid::comparable(sid const &id) const {
	if (!valid() || !id.valid()) {
		return 0;
	}
	if (branch != id.branch) {
		return 0;
	}
	if (branch != 0 && rel != id.rel && level != id.level) {
		return 0;
	}
	return 1;
}

int
sid::gt(sid const &id) const {
	if (rel > id.rel) {
		return 1;
	}
	if (rel != id.rel) {
		return 0;
	}
	if (level > id.level) {
		return 1;
	}
	if (level != id.level) {
		return 0;
	}
#if 0
	if (branch > id.branch) {
		return 1;
	}
	if (branch != id.branch) {
		return 0;
	}
#endif
	return sequence > id.sequence;
}

int
sid::gte(sid const &id) const {
	if (rel > id.rel) {
		return 1;
	}
	if (rel != id.rel) {
		return 0;
	}
	if (level > id.level) {
		return 1;
	}
	if (level != id.level) {
		return 0;
	}
#if 0
	if (branch > id.branch) {
		return 1;
	}
	if (branch != id.branch) {
		return 0;
	}
#endif
	return sequence >= id.sequence;
}

int
sid::partial_match(sid const &id) const {
	if (!comparable(id)) {
		return 0;
	}

	if (rel == 0) {
		return 1;
	}
	if (rel != id.rel) {
		return 0;
	}
	if (level == 0) {
		return 1;
	}
	if (level != id.level) {
		return 0;
	}
	if (branch == 0) {
		return 1;
	}
	if (branch != id.branch) {
		return 0;
	}
	return sequence == 0 || sequence == id.rel;
}

sid
sid::successor() const {
	if (is_null()) {
		return sid(1, 1, 0, 0);
	} else if (branch != 0) {
		return sid(rel, level, branch, sequence + 1);
	} else {
		return sid(rel, level + 1, 0, 0);
	}
}

int sid::components() const
{
  if (valid() && rel)
    if (level)
      if (branch)
	if (sequence)
	  return   4;
	else
	  return 3;
      else
	return 2;
    else
      return 1;
  else
    return 0;
}

bool sid::on_trunk() const
{
  return 2 == components();
}

bool sid::matches(const sid &m, int nfields) const
{
  if (0 == nfields--)
    return true;
  if (rel != m.rel)
    return false;
  
  if (0 == nfields--)
    return true;
  if (level != m.level)
    return false;
  
  if (0 == nfields--)
    return true;
  if (branch != m.branch)
    return false;
  
  if (0 == nfields--)
    return true;
  if (sequence != m.sequence)
    return false;
  
  return true;
}


int
sid::print(FILE *out) const {
	ASSERT(valid());
	ASSERT(rel != 0);

	if (printf_failed(fprintf(out, "%d", rel))
	    || (level != 0 
		&& (printf_failed(fprintf(out, ".%d", level))
	            || (branch != 0
			&& (printf_failed(fprintf(out, ".%d", branch))
			    || (sequence != 0
				&& printf_failed(fprintf(out, ".%d",
							 sequence))))))))
	  {
	    return 1;
	  }
	return 0;
}

int
sid::printf(FILE *out, char c, int force_zero /*=0*/) const {
	ASSERT(valid());
	ASSERT(!partial_sid());

	short n;

	switch (c) {
	case 'R':
		n = rel;
		break;

	case 'L':
		n = level;
		break;

	case 'B':
	        // this field is completely blank for trunk revisions.
                if (!force_zero && 0 == branch && 0 == sequence)
		  return 0;
		n = branch;
		break;

	case 'S':
	        // this field is completely blank for trunk revisions.
                if (!force_zero && 0 == branch && 0 == sequence)
		  return 0;
		n = sequence;
		break;

	default:
          ASSERT(0);
	}
	return printf_failed(fprintf(out, "%d", n));
}

relvbr::relvbr(const sid &s) :  rel( (short)s.rel ), level( (short)s.level ), branch( (short)s.branch )
{
  // nothing.
}

release::release(const sid &s) :  rel( (short)s.rel )
{
  // nothing.
}


/* Local variables: */
/* mode: c++ */
/* End: */
