/*
 * sid.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Defines the classes sid, and release as well as the typedef's sid_list
 * and release_list.  
 *
 * @(#) CSSC sid.h 1.1 93/11/09 17:17:51
 *
 */

#ifndef __SID_H__
#define __SID_H__

#ifdef __GNUC__
#pragma interface
#endif

#include "sid_list.h"

class release;

class sid {
	short rel, level, branch, sequence;

	int comparable(sid const &id) const;
	int gt(sid const &id) const;
	int gte(sid const &id) const;

	sid(short r, short l, short b, short s)
		: rel(r), level(l), branch(b), sequence(s) {
		assert((!r && !l && !b && !s)
		       || (r && !l && !b && !s)
		       || (r && l && !b && !s)
		       || (r && l && b));
	}

public:
	sid(): rel(-1) {}
	sid(const char *s);
	sid(release);		/* Defined below */

  	bool is_null() const { return rel <= 0; }

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

	operator void const *() const {
		if (rel == 0)  {
			return NULL;
		}
		return (void const *) this;
	}

	operator release() const;	/* Defined below */

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
	int printf(FILE *f, char fmt) const;

	void
	dprint(FILE *f) const {
		fprintf(f, "%d.%d.%d.%d", rel, level, branch, sequence);
	}
};

class release {
	friend sid::operator release() const;
	friend sid::sid(release);

	short rel;

	release(short r, sid const *): rel(r) {}

public:
	release(): rel(-1) {}
	release(const char *s);

	int valid() const { return rel > 0; }

	operator void const *() const {
		if (rel == 0) {
			return NULL;
		} else {
			return (void const *) this;
		}
	}

	release &operator++() { rel++; return *this; }
	release &operator--() { rel--; return *this; }

	friend int operator <(release r1, release r2) {
		return r1.rel < r2.rel;
	}

	friend int operator >(release r1, release r2) {
		return r1.rel > r2.rel;
	}

	friend int operator <=(release r1, release r2) {
		return r1.rel <= r2.rel;
	}

	friend int operator >=(release r1, release r2) {
		return r1.rel >= r2.rel;
	}

	friend int operator ==(release r1, release r2) {
		return r1.rel == r2.rel;
	}

	friend int operator !=(release r1, release r2) {
		return r1.rel != r2.rel;
	}

	int print(FILE *out) const { return fprintf(out, "%d", rel); }
};

inline sid::sid(release r): rel(r.rel), level(0), branch(0), sequence(0) {}
inline sid::operator release() const { return release(rel, this); }

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

#pragma warn -inl
typedef range_list<release> release_list;
typedef range_list<sid> sid_list;
#pragma warn .inl

#endif /* __SID_H__ */
	
/* Local variables: */
/* mode: c++ */
/* End: */

