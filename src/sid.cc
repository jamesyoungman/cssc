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

relvbr::relvbr(const sid &s)
  :relvbr(s.get_relvbr())
{
  // nothing.
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
  : rel_(-1), level_(0), branch_(0), sequence_(0) {
        ASSERT(s != NULL);
	rel_ = get_comp(s);
	level_ = get_comp(s);
	branch_ = get_comp(s);
	sequence_ = get_comp(s);

	if (*s != '\0' || rel_ == 0 || sequence_ == -1)
	  {
	    rel_ = -1;
	  }
}

int
sid::comparable(sid const &id) const
{
  if (!valid() || !id.valid())
    {
      return 0;
    }
  if (branch_ != id.branch_)
    {
      return 0;
    }
  if (branch_ != 0 && rel_ != id.rel_ && level_ != id.level_)
    {
      return 0;
    }
  return 1;
}

int
sid::gt(sid const &id) const {
	if (rel_ > id.rel_)
	  {
	    return 1;
	  }
	if (rel_ != id.rel_)
	  {
	    return 0;
	  }
	if (level_ > id.level_)
	  {
	    return 1;
	  }
	if (level_ != id.level_)
	  {
	    return 0;
	  }
#if 0
	if (branch_ > id.branch_)
	  {
		return 1;
	}
	if (branch_ != id.branch_)
	  {
		return 0;
	}
#endif
	return sequence_ > id.sequence_;
}

bool
sid::gte(sid const &id) const
{
  if (rel_ > id.rel_)
    {
      return true;
    }
  if (rel_ != id.rel_)
    {
      return false;
    }
  if (level_ > id.level_)
    {
      return true;
    }
  if (level_ != id.level_)
    {
      return false;
    }
#if 0
  // XXX: why is this code commented out?
  if (branch_ > id.branch_)
    {
      return true;
    }
  if (branch_ != id.branch_)
    {
      return false;
    }
#endif
  return sequence_ >= id.sequence_;
}

bool
sid::partial_match(sid const &id) const
{
  if (!comparable(id))
    {
      return false;
    }

  if (rel_ == 0)
    {
      return true;
    }
  if (rel_ != id.rel_)
    {
      return false;
    }
  if (level_ == 0)
    {
      return true;
    }
  if (level_ != id.level_)
    {
      return false;
    }
  if (branch_ == 0)
    {
      return true;
    }
  if (branch_ != id.branch_)
    {
      return false;
    }
  // TODO: shouldn't this be sequence_ == id.rel_?
  return sequence_ == 0 || sequence_ == id.rel_;
}

sid
sid::successor() const
{
  if (is_null())
    {
      return sid(1, 1, 0, 0);
    }
  else if (branch_ != 0)
    {
      short next_seq = sequence_;
      ++next_seq;
      return sid(rel_, level_, branch_, next_seq);
    }
  else
    {
      short next_lev = level_;
      ++next_lev;
      return sid(rel_, next_lev, 0, 0);
    }
}

int sid::components() const
{
  if (valid() && rel_)
    if (level_)
      if (branch_)
	if (sequence_)
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

  if (rel_ != m.rel_)
    return false;

  if (0 == nfields)
    return true;
  else
    --nfields;

  if (level_ != m.level_)
    return false;

  if (0 == nfields)
    return true;
  else
    --nfields;

  if (branch_ != m.branch_)
    return false;

  if (0 == nfields)
    return true;
  else
    --nfields;

  if (sequence_ != m.sequence_)
    return false;

  return true;
}

release sid::get_release() const
{
  return rel_;
}

relvbr sid::get_relvbr() const
{
  return relvbr(rel_, level_, branch_);
}


cssc::Failure
sid::print(FILE *out) const
{
  ASSERT(valid());
  ASSERT(rel_ != 0);

  if (printf_failed(fprintf(out, "%d", rel_))
      || (level_ != 0
	  && (printf_failed(fprintf(out, ".%d", level_))
	      || (branch_ != 0
		  && (printf_failed(fprintf(out, ".%d", branch_))
		      || (sequence_ != 0
			  && printf_failed(fprintf(out, ".%d",
						   sequence_))))))))
    {
      return cssc::make_failure_from_errno(errno);
    }
  return cssc::Failure::Ok();
}


std::ostream& sid::ostream_insert(std::ostream& os) const
{
  os << rel_;
  if (level_)
    {
      os << '.' << level_;
      if (branch_)
	{
	  os << '.' << branch_;
	  if (sequence_)
	    {
	      os << '.' << sequence_;
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
		n = rel_;
		break;

	case 'L':
		n = level_;
		break;

	case 'B':
	        // this field is completely blank for trunk revisions.
                if (!force_zero && 0 == branch_ && 0 == sequence_)
		  return cssc::Failure::Ok();
		n = branch_;
		break;

	case 'S':
	        // this field is completely blank for trunk revisions.
                if (!force_zero && 0 == branch_ && 0 == sequence_)
		  return cssc::Failure::Ok();
		n = sequence_;
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

release::release(const sid &s)
  :  rel(s.get_release())
{
  // nothing.
}


/* Local variables: */
/* mode: c++ */
/* End: */
