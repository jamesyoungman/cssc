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
static const char rcs_id[] = "CSSC $Id: sccsdate.cc,v 1.9 1997/11/21 19:36:24 james Exp $";
#endif

// The MySC code used to just check for (year % 4) and (year == 0).
// This implementation is "right", but in any case it won't make a
// difference until 2100AD.
static int
is_leapyear(int year)
{
  if (year % 4)
    {
      return 0;			// not a leapyear.
    }
  else 
    {
      if (year % 100)
	{
	  return 1;		// non-century year
	}
      else
	{
	  if (year % 400)
	    return 0;		// century years are not leap-years exceot
	  else
	    return 1;		// every fourth one, which IS a leap year.
	}
    }
}

inline int 
get_digit(char ch)
{
  ASSERT(isdigit( (unsigned char) ch ));
  return ch - '0';
}

static int
days_in_month(int mon, int year)
{
  switch(mon)
    {
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
      if (is_leapyear(year + 1900))
	return 29;
      else
	return 28;
    }
  return -1;
}

static int
get_part(const char *&s, int def)
{
  char c = *s;
  while (c)
    {
      if (isdigit(c))
	{
	  s++;
	  if (isdigit(*s))
	    return get_digit(c) * 10 + get_digit(*s++);
	  else
	    return get_digit(c);
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

static int 
count_digits(const char *s)
{
  int count;
  
  for(count=0; *s; s++)
    {
      if (isdigit((unsigned char)*s))
	count++;
    }
  return count;
}

// Construct a date as specified on the command line.
sccs_date::sccs_date(const char *s) {
	t = (time_t) -1;

	if (s == 0)
	  return;

	struct tm tm;


	// A fully-qualified YYmmDDhhMMss specification contains
	// twelve (12) digits.  If we have more than that, assume that
	// the first two digits are the century.  We have to make this
	// count before the first call to get_part(), since that
	// actually advances the pointer.
	const int n_digits = count_digits(s);
	
	// Get and check the year part.
	if ( (tm.tm_year = get_part(s, -1)) == -1)
	  return;

#if 1
	// Here be Year-2000 code.
	//
	// This code checks if the year is 19 or 20 AND the next two
	// characters are ALSO digits.  If so, we assume that we've
	// just got the century.  This is an extension with respect to
	// "real" SCCS.
	//
	// To prevent this simply breaking down in the year 2019 (with
	// a two digit year part), we only activate this measure if we
	// have more than the regulation number of digits.
	//
	// The version of MySC which I inherited assumed the first 2
	// digits to be the century part if they were 19 or 20.  This
	// approach breaks down rather unexpectedly for the user in
	// the year 2019.
	// 
	if (n_digits > 12)
	  {
	    // If we actually have a 4-digit year, the next two
	    // characters must be digits (we have already consumed the
	    // first two).
	    if (isdigit((unsigned char)s[0]) &&
		isdigit((unsigned char)s[1]))
	      {
		// tm_year needs to be set to the number of years
		// since 1900.
		const int century_field_val = (tm.tm_year - 19);
		tm.tm_year =  century_field_val * 100
		  + get_digit(s[0]) * 10 + get_digit(s[1]);
		s += 2;		// this consumes exactly two characters.
	      }
	  }
	else
	  {
	    // In the X/Open Commands and Utilities Issue 5 standard,
	    // it is specified that yy/mm/dd type dates with values
	    // for "yy" which range from 69-99 are to be interpreted
	    // as 1969-1999 and dates with year values 00-68 are to be
	    // interpreted as 2000-2068.
	    //
	    // For more information about this, please see that
	    // document itself and also the X/Open Year-2000 FAQ,
	    // which can be obtained from the URL
	    // http://www.xopen.org/public/tech/base/year2000.html
	    //
	    if (tm.tm_year < 69)
	      tm.tm_year += 100;
	  }
#endif
			
	tm.tm_mon  = get_part(s, 12) - 1;
	tm.tm_mday = get_part(s, days_in_month(tm.tm_mon + 1, tm.tm_year));
	tm.tm_hour = get_part(s, 23);
	tm.tm_min  = get_part(s, 59);
	tm.tm_sec  = get_part(s, 59);

	tm.tm_isdst = -1;	// i.e. "don't know".

	if (check_tm(tm)) {
		return;
	}

	t = mktime(&tm);
}

// Construct a date as specified in an SCCS file.
sccs_date::sccs_date(const char *date, const char *time) {
	struct tm tm;

	t = (time_t) -1;

	if (!isdigit(date[0]) || !isdigit(date[1]) || date[2] != '/') {
		return;
	}
	tm.tm_year = (date[0] - '0') * 10 + (date[1] - '0');


	// Year 2000 fix (mandated by X/Open white paper, see above
	// for more details).
	if (tm.tm_year < 69)
	  tm.tm_year += 100;



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

	tm.tm_isdst = -1;	// i.e. "don't know".

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
		ASSERT(!"sccs_date::printf: Invalid format");
	}

	return 0;
}

int
sccs_date::print(FILE *f) const
{
  return this->printf(f, 'D')
    || putc_failed(putc(' ', f))
    || this->printf(f, 'T');
}


// LOCAL time really is the right thing to use, apparently.
const char *
sccs_date::as_string() const {
	struct tm const *tm = localtime(&t); 

	if (tm == NULL) {
		quit(-1, "localtime() failed.");
	}

	char buf[18];

	ASSERT(tm->tm_mon >= 0);
	ASSERT(tm->tm_mon < 12); // months are counted from zero (!)

	ASSERT(tm->tm_mday > 0);
	ASSERT(tm->tm_mday < 32);

	ASSERT(tm->tm_hour >= 0);
	ASSERT(tm->tm_hour < 24);

	ASSERT(tm->tm_min >= 0);
	ASSERT(tm->tm_min < 60);

	/* The ANSI standard allows tm_sec to be as large as
	 * 61, but in fact there will never be TWO leap-seconds
	 * in the same minute (at least not with our current
	 * UTC system unless _really_ weird things happen to
	 * the Earth's orbit).  However I don't have copies
	 * all the addenda to the C standard, but just the
	 * oridinal document.  Hence this assertion for the
	 * moment stays at 61.
	 */
	ASSERT(tm->tm_sec < 61);
	
	
	sprintf(buf, "%02d/%02d/%02d %02d:%02d:%02d",
		tm->tm_year % 100, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	return buf;
}

/* Local variables: */
/* mode: c++ */
/* End: */
