/*
 * sid.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1999, 2007, 2008, 2009, 2010, 2011, 2014, 2019
 *  Free Software Foundation, Inc.
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
 * Members of the classes sid and release.
 *
 */

#include <config.h>
#include <string>

#include <ctype.h>

#include "cssc.h"
#include "sid.h"
#include "ioerr.h"

std::string sid::as_string() const
{
  std::ostringstream oss;
  oss << (*this);
  return oss.str();
}

static short int
get_comp(const char *&s) {
	int n = 0;
	char c = *s;
	while (c != '\0') {
		if (c == '.') {
			if (n == 0) {
			  return short(-1);
			}
			s++;
			return static_cast<short int>(n);
		}
		if (isdigit(c)) {
			n = n * 10 + (c - '0');
		} else {
		  return short(-1);
		}
		c = *++s;
	}
	return static_cast<short int>(n);
}

relvbr::relvbr(const char *s)
  : relvbr() {
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

release::release(const char *s)
  : release() {
	if (s == NULL) {
	        // TODO: the default constructor sets rel=-1.
	        // Is there a meaningful difference here?
		rel = 0;
		return;
	}

	rel = get_comp(s);

	if (*s != '\0' || rel == 0) {
		rel = -1;
	}
}

sid
sid::null_sid()
{
  return sid(0, 0, 0, 0);
}

sid::sid(const char *s)
  : rel(-1), level(0), branch(0), sequence(0) {
        ASSERT(s != NULL);
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

bool
sid::gte(sid const &id) const {
	if (rel > id.rel) {
		return true;
	}
	if (rel != id.rel) {
		return false;
	}
	if (level > id.level) {
		return true;
	}
	if (level != id.level) {
		return false;
	}
#if 0
	// XXX: why is this code commented out?
	if (branch > id.branch) {
		return true;
	}
	if (branch != id.branch) {
		return false;
	}
#endif
	return sequence >= id.sequence;
}

bool
sid::partial_match(sid const &id) const {
	if (!comparable(id)) {
		return false;
	}

	if (rel == 0) {
		return true;
	}
	if (rel != id.rel) {
		return false;
	}
	if (level == 0) {
		return true;
	}
	if (level != id.level) {
		return false;
	}
	if (branch == 0) {
		return true;
	}
	if (branch != id.branch) {
		return false;
	}
	return sequence == 0 || sequence == id.rel;
}

sid
sid::successor() const {
	if (is_null()) {
		return sid(1, 1, 0, 0);
	} else if (branch != 0) {
	        short next_seq = sequence;
		++next_seq;
		return sid(rel, level, branch, next_seq);
	} else {
	        short next_lev = level;
		++next_lev;
		return sid(rel, next_lev, 0, 0);
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
  if (0 == nfields)
    return true;
  else
    --nfields;

  if (rel != m.rel)
    return false;

  if (0 == nfields)
    return true;
  else
    --nfields;

  if (level != m.level)
    return false;

  if (0 == nfields)
    return true;
  else
    --nfields;

  if (branch != m.branch)
    return false;

  if (0 == nfields)
    return true;
  else
    --nfields;

  if (sequence != m.sequence)
    return false;

  return true;
}

release sid::get_release() const
{
  return rel;
}



cssc::Failure
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
	    return cssc::make_failure_from_errno(errno);
	  }
	return cssc::Failure::Ok();
}


std::ostream& sid::ostream_insert(std::ostream& os) const
{
  os << rel;
  if (level)
    {
      os << '.' << level;
      if (branch)
	{
	  os << '.' << branch;
	  if (sequence)
	    {
	      os << '.' << sequence;
	    }
	}
    }
  return os;
}


cssc::Failure
sid::printf(FILE *out, char c, bool force_zero /*=false*/) const {
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
		  return cssc::Failure::Ok();
		n = branch;
		break;

	case 'S':
	        // this field is completely blank for trunk revisions.
                if (!force_zero && 0 == branch && 0 == sequence)
		  return cssc::Failure::Ok();
		n = sequence;
		break;

	default:
          ASSERT(0);
	}
	if (printf_failed(fprintf(out, "%d", n)))
	  {
	    return cssc::make_failure_from_errno(errno);
	  }
	return cssc::Failure::Ok();
}

relvbr::relvbr(const sid &s)
  : rel(s.rel),
    level(s.level),
    branch(s.branch)
{
  // nothing.
}

release::release(const sid &s)
  :  rel(s.rel)
{
  // nothing.
}


/* Local variables: */
/* mode: c++ */
/* End: */
