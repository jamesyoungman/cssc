/*
 * sccsdate.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Defines the class sccs_date.
 *
 * @(#) CSSC sccsdate.h 1.1 93/11/09 17:17:49
 *
 */

#ifndef __SCCSDATE_H__
#define __SCCSDATE_H__

#include <time.h>

#ifdef __GNUC__
#pragma interface
#endif

class sccs_date {
	time_t t;
	
	sccs_date(int, int) {}

public:
	sccs_date(): t((time_t) -1) {}
	sccs_date(const char *cutoff);
	sccs_date(const char *date, const char *time);

	int valid() const { return t != -1; }

	static sccs_date now() {
		sccs_date temp(0, 0);
		time(&temp.t);
		return temp;
	}

	const char *as_string() const;

	int printf(FILE *f, char fmt) const;
	int print(FILE *f) const;

#ifdef CONFIG_USE_DIFFTIME
	int
	operator >(sccs_date const &d) const {
		return difftime(t, d.t) > 0;
	}

	int
	operator <(sccs_date const &d) const {
		return difftime(t, d.t) < 0;
	}

	int
	operator >=(sccs_date const &d) const {
		return difftime(t, d.t) >= 0;
	}

	int
	operator <=(sccs_date const &d) const {
		return difftime(t, d.t) <= 0;
	}

	int
	operator ==(sccs_date const &d) const {
		return difftime(t, d.t) == 0;
	}

	int
	operator !=(sccs_date const &d) const {
		return difftime(t, d.t) != 0;
	}

#else /* CONFIG_USE_DIFFTIME */

	int operator >(sccs_date const &d) const { return t > d.t; }
	int operator <(sccs_date const &d) const { return t < d.t; }
	int operator >=(sccs_date const &d) const { return t >= d.t; }
	int operator <=(sccs_date const &d) const { return t <= d.t; }
	int operator ==(sccs_date const &d) const { return t == d.t; }
	int operator !=(sccs_date const &d) const { return t != d.t; }

#endif /* CONFIG_USE_DIFFTIME */
};


#endif /* __SCCSDATE_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
