/*
 * sid.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Members of the classes sid and release.
 *
 */

#ifdef __GNUC__
#pragma implementation "sid.h"
#pragma implementation "sid_list.h"
#pragma implementation "sid_list.c"
#endif

#include "cssc.h"
#include "sid.h"

#include <ctype.h>

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sid.cc,v 1.3 1997/05/10 14:49:58 james Exp $";
#endif

/* This pointer is used by the template range_list to denote an
   invalid range. */

void *invalid_range = xmalloc(1);

static int
get_comp(const char *&s) {
	int n = 0;
	char c = *s;
	while(c != '\0') {
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

release::release(const char *s) {
	if (s == NULL) {
		rel = 0;
		return;
	}

	rel = get_comp(s);

	if (*s != 0 || rel == 0) {
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

int
sid::print(FILE *out) const {
	assert(valid());
	assert(rel != 0);

	if (fprintf(out, "%d", rel) == EOF
	    || (level != 0 
		&& (fprintf(out, ".%d", level) == EOF
	            || (branch != 0
			&& (fprintf(out, ".%d", branch) == EOF
			    || (sequence != 0
				&& fprintf(out, ".%d", sequence) == EOF)))))) {
		return 1;
	}
	return 0;
}

int
sid::printf(FILE *out, char c) const {
	assert(valid());
	assert(!partial_sid());

	short n;

	switch(c) {
	case 'R':
		n = rel;
		break;

	case 'L':
		n = level;
		break;

	case 'B':
		n = branch;
		break;

	case 'S':
		n = sequence;
		break;

	default:
		quit(-1, "sid::printf: Invalid format.");
	}
	return fprintf(out, "%d", n) == EOF;
}

/* Local variables: */
/* mode: c++ */
/* End: */
