/*
 * sid.h: Part of GNU CSSC.
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
 * Defines the classes sid, and release as well as the typedef's sid_list
 * and release_list.  
 *
 * @(#) CSSC sid.h 1.1 93/11/09 17:17:51
 *
 */

#ifndef CSSC__SID_H__
#define CSSC__SID_H__

#include "sid_list.h"

#include "release.h"

#include "relvbr.h"

class sccs_file;

class sid {
	short rel, level, branch, sequence;

	int comparable(sid const &id) const;
	int gt(sid const &id) const;

	sid(short r, short l, short b, short s)
		: rel(r), level(l), branch(b), sequence(s) {
		ASSERT((!r && !l && !b && !s)
		       || (r && !l && !b && !s)
		       || (r && l && !b && !s)
		       || (r && l && b));
	}

public:
	sid(): rel(-1), level(0), branch(0), sequence(0) {}
	sid(const char *s);
	sid(release);		/* Defined below */
	sid(relvbr);		/* Defined below */

  	bool is_null() const { return rel <= 0; }
	int gte(sid const &id) const; // used by sccs_file::find_requested_sid().

#if 1
	sid(sid const &id): rel(id.rel), level(id.level),
			    branch(id.branch), sequence(id.sequence) {}

	sid &
	operator =(sid const &id) {
		rel = id.rel;
		level = id.level;
		branch = id.branch;
		sequence = id.sequence;
		return *this;
	}
#endif

	bool valid() const { return rel > 0; }

	int
	partial_sid() const {
		return level == 0 || (branch != 0 && sequence == 0);
	}
	int components() const;
  	bool on_trunk() const;
  
	operator void const *() const {
		if (rel == 0)  {
			return NULL;
		}
		return (void const *) this;
	}

  //	operator release() const;	/* Defined below */

	friend int
	operator >(sid const &i1, sid const &i2) {
		return i1.comparable(i2) && i1.gt(i2);
	}

	friend int
	operator >=(sid const &i1, sid const &i2) {
		return i1.comparable(i2) && i1.gte(i2);
	}

	friend int
	operator <(sid const &i1, sid const &i2) {
		return i1.comparable(i2) && !i1.gte(i2);
	}

	friend int
	operator <=(sid const &i1, sid const &i2) {
		return i1.comparable(i2) && !i1.gt(i2);
	}

	friend int
	operator ==(sid const &i1, sid const &i2) {
		return memcmp(&i1, &i2, sizeof(sid)) == 0;
	}

	friend int
	operator !=(sid const &i1, sid const &i2) {
		return memcmp(&i1, &i2, sizeof(sid)) != 0;
	}

	sid successor() const;

	sid &
	next_branch() {
		branch++;
		sequence = 1;
		return *this;
	}

	const sid &
	next_level() {
		++level;
		branch = sequence = 0;
		return *this;
	}

	sid &
	operator++() { 
		if (branch != 0) {
			sequence++;
		} else if (level != 0) {
			level++;
		} else {
			rel++;
		}
		return *this;
	}

	sid &
	operator--() {
		if (branch != 0) {
			sequence--;
		} else if (level != 0) {
			level--;
		} else {
			rel--;
		}
		return *this;
	}

	int
	is_trunk_successor(sid const &id) const {
		return branch == 0 && *this < id;
	}

	int
	branch_greater_than(sid const &id) const {
		return rel == id.rel && level == id.level
		       && branch > id.branch;
	}

	int partial_match(sid const &id) const;
  	bool matches(const sid &m, int nfields) const;

	int
	release_only() const {
		return rel != 0 && level == 0;
	}

	int
	trunk_match(sid const &id) const {
		return rel == 0 
		       || (rel == id.rel && (level == 0
					     || level == id.level));
	}

	int print(FILE *f) const;
	int printf(FILE *f, char fmt, int force_zero=0) const;

  	int			// 0 return means success.
	dprint(FILE *f) const {
		return EOF == fprintf(f, "%d.%d.%d.%d",
				      rel, level, branch, sequence);
	}
  
  	mystring as_string() const;

  friend release::release(const sid &s);
  friend relvbr::relvbr(const sid &s);
};


inline sid::sid(release r): rel(r), level(0), branch(0), sequence(0) {}
//inline sid::operator release() const { return release(rel); }

#if 1

inline int operator >(release i1, sid const &i2) { return i1 > release(i2); }
inline int operator <(release i1, sid const &i2) { return i1 < release(i2); }
inline int operator >=(release i1, sid const &i2) { return i1 >= release(i2); }
inline int operator <=(release i1, sid const &i2) { return i1 <= release(i2); }
inline int operator ==(release i1, sid const &i2) { return i1 == release(i2); }
inline int operator !=(release i1, sid const &i2) { return i1 != release(i2); }

inline int operator >(sid const &i1, release i2) { return release(i1) > i2; }
inline int operator <(sid const &i1, release i2) { return release(i1) < i2; }
inline int operator >=(sid const &i1, release i2) { return release(i1) >= i2; }
inline int operator <=(sid const &i1, release i2) { return release(i1) <= i2; }
inline int operator ==(sid const &i1, release i2) { return release(i1) == i2; }
inline int operator !=(sid const &i1, release i2) { return release(i1) != i2; }

#endif

typedef range_list<sid> sid_list;


#endif /* __SID_H__ */
	
/* Local variables: */
/* mode: c++ */
/* End: */

