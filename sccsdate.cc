/*
 * sccsdate.c: Part of GNU CSSC.
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
 * Members of the class sccs_date.
 *
 */

#ifdef __GNUC__
#pragma implementation "sccsdate.h"
#endif

#include "cssc.h"
#include "sccsdate.h"

#include <ctype.h>

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sccsdate.cc,v 1.7 1997/11/15 20:05:45 james Exp $";
#endif

static int
days_in_month(int mon, int year) {
	switch(mon) {
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		return 31;

	case 4:
	case 6:
	case 9:
	case 11:
		return 30;

	case 2:
		if (year % 4 == 0 && year != 0) {
			return 29;
		}
		return 28;
	}
	return -1;
}

static int
get_part(const char *&s, int def) {
	char c = *s;
	while(c != '\0') {
		if (isdigit(c)) {
			s++;
			if (isdigit(*s)) {
				return (c - '0') * 10 + (*s++ - '0');
			}
			return c - '0';
		}
		c = *++s;
	}
	return def;
}

static int
check_tm(struct tm &tm) {
	if (tm.tm_year < 69) {
		tm.tm_year += 100;
	}

	return tm.tm_mon == -1 || tm.tm_mon > 11
	       || tm.tm_mday > days_in_month(tm.tm_mon + 1, tm.tm_year)
	       || tm.tm_hour > 23 || tm.tm_min > 59 || tm.tm_sec > 59;
}

sccs_date::sccs_date(const char *s) {
	t = (time_t) -1;

	if (s == NULL) {
		return;
	}

	struct tm tm;

	tm.tm_year = get_part(s, -1);
	if (tm.tm_year == -1) {
		return;
	}

#if 1
	if ((tm.tm_year == 19 || tm.tm_year == 20)
            && isdigit(s[0]) && isdigit(s[1])) {
		tm.tm_year = (tm.tm_year - 19) * 100
			     + (s[0] - '0') * 10 + (s[1] - '0');
		s += 2;
	}
#endif
			
	tm.tm_mon = get_part(s, 12) - 1;
	tm.tm_mday = get_part(s, days_in_month(tm.tm_mon + 1, tm.tm_year));
	tm.tm_hour = get_part(s, 23);
	tm.tm_min = get_part(s, 59);
	tm.tm_sec = get_part(s, 59);

	tm.tm_isdst = -1;

	if (check_tm(tm)) {
		return;
	}

	t = mktime(&tm);
}

sccs_date::sccs_date(const char *date, const char *time) {
	struct tm tm;

	t = (time_t) -1;

	if (!isdigit(date[0]) || !isdigit(date[1]) || date[2] != '/') {
		return;
	}
	tm.tm_year = (date[0] - '0') * 10 + (date[1] - '0');

	if (!isdigit(date[3]) || !isdigit(date[4]) || date[5] != '/') {
		return;
	}
	tm.tm_mon = (date[3] - '0') * 10 + (date[4] - '0') - 1;

	if (!isdigit(date[6]) || !isdigit(date[7]) || date[8] != '\0') {
		return;
	}
	tm.tm_mday = (date[6] - '0') * 10 + (date[7] - '0');

	if (!isdigit(time[0]) || !isdigit(time[1]) || time[2] != ':') {
		return;
	}
	tm.tm_hour = (time[0] - '0') * 10 + (time[1] - '0');

	if (!isdigit(time[3]) || !isdigit(time[4]) || time[5] != ':') {
		return;
	}
	tm.tm_min = (time[3] - '0') * 10 + (time[4] - '0');

	if (!isdigit(time[6]) || !isdigit(time[7]) || time[8] != '\0') {
		return;
	}
	tm.tm_sec = (time[6] - '0') * 10 + (time[7] - '0');

	tm.tm_isdst = -1;

	if (check_tm(tm)) {
		return;
	}

	t = mktime(&tm);
}

int
sccs_date::printf(FILE *f, char fmt) const {
	struct tm const *tm = localtime(&t); 

	if (tm == NULL) {
		quit(-1, "localtime() failed.");
	}

	switch(fmt) {
	case 'D':
		return printf_failed(fprintf(f, "%02d/%02d/%02d",
					     tm->tm_year % 100,
					     tm->tm_mon + 1,
					     tm->tm_mday) );

	case 'H':
		return printf_failed(fprintf(f, "%02d/%02d/%02d",
					     tm->tm_mon + 1,
					     tm->tm_mday,
					     tm->tm_year % 100));

	case 'T':
		return printf_failed(fprintf(f, "%02d:%02d:%02d",
					     tm->tm_hour,
					     tm->tm_min, tm->tm_sec));

	case 'y':
		return printf_failed(fprintf(f, "%02d", tm->tm_year % 100));
	
	case 'o':
		return printf_failed(fprintf(f, "%02d", tm->tm_mon + 1));
		
	case 'd':
		return printf_failed(fprintf(f, "%02d", tm->tm_mday));

	case 'h':
		return printf_failed(fprintf(f, "%02d", tm->tm_hour));

	case 'm':
		return printf_failed(fprintf(f, "%02d", tm->tm_min));

	case 's':
		return printf_failed(fprintf(f, "%02d", tm->tm_sec));

	default:
		assert(!"sccs_date::printf: Invalid format");
	}

	return 0;
}

int
sccs_date::print(FILE *f) const {
	return
	  printf_failed(printf(f, 'D')) ||
	  putc_failed(putc(' ', f)) ||
	  printf_failed(printf(f, 'T'));
}


const char *
sccs_date::as_string() const {
	struct tm const *tm = localtime(&t); 

	if (tm == NULL) {
		quit(-1, "localtime() failed.");
	}

	static char buf[18];

	sprintf(buf, "%02d/%02d/%02d %02d:%02d:%02d",
		tm->tm_year % 100, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	return buf;
}

/* Local variables: */
/* mode: c++ */
/* End: */
