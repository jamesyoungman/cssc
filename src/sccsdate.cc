/*
 * sccsdate.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 2007, 2008, 2009, 2010, 2011, 2014, 2019, 2024
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
 * Members of the class sccs_date.
 *
 */
#include <config.h>
#include <cstring>
#include <string>
#include <memory>

#include "cssc.h"
#include "except.h"
#include "sccsdate.h"
#include "ioerr.h"
#include "failure.h"
#include "cssc-assert.h"

#include <time.h>

#include <ctype.h>

namespace
{
  int y2k_window(int year)
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
    if (year < 69)
      year += 2000;
    else
      year += 1900;

    ASSERT(year >= 1969);
    ASSERT(year <  2069);
    return year;
  }
}

// The MySC code used to just check for (year % 4) and (year == 0).
// This implementation is "right", but in any case it won't make a
// difference until 2100AD.
static int
is_leapyear(int year)
{
  if (year % 4)
    {
      return 0;                 // not a leapyear.
    }
  else
    {
      if (year % 100)
        {
          return 1;             // non-century year
        }
      else
        {
          if (year % 400)
            return 0;           // century years are not leap-years except
          else
            return 1;           // every fourth one, which IS a leap year.
        }
    }
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
      if (is_leapyear(year))
        return 29;
      else
        return 28;
    }
  return -1;
}

static inline int
is_digit(char ch)
{
  return isdigit(static_cast<unsigned char>(ch));
}

static inline int
get_digit(char ch)
{
  ASSERT(isdigit(static_cast<unsigned char>(ch)));
  return ch - '0';
}

static int
get_two_digits(const char *s)
{
  return get_digit(s[0]) * 10 + get_digit(s[1]);
}

static int
get_two_digits(const std::string& s, size_t pos)
{
  return get_digit(s[pos]) * 10 + get_digit(s[pos+1]);
}

static int
get_part(const char *&s, int def)
{
  char c = *s;
  while (c)
    {
      if (is_digit(c))
        {
          s++;
          if (is_digit(*s))
            {
              return get_digit(c) * 10 + get_digit(*s++);
            }
          else
            {
              return get_digit(c);
            }
        }
      c = *++s;
    }
  return def;
}

static int
count_digits(const char *s)
{
  int count;

  for(count=0; *s; s++)
    {
      if (is_digit(*s))
        count++;
    }
  return count;
}

// Construct a date as specified on the command line.
sccs_date::sccs_date(const char *s)
  : year_(-1), month_(-1), month_day_(-1),
    hour_(-1), minute_(-1), second_(-1),
    yearday_(-1)
{
  ASSERT(s != nullptr);
  /* if (s == 0) return; */

  // A fully-qualified YYmmDDhhMMss specification contains
  // twelve (12) digits.  If we have more than that, assume that
  // the first two digits are the century.  We have to make this
  // count before the first call to get_part(), since that
  // actually advances the pointer.
  const int n_digits = count_digits(s);

  // Get and check the year part.
  if ( (year_ = get_part(s, -1)) == -1)
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
      if (isdigit(static_cast<unsigned char>(s[0])) &&
          isdigit((static_cast<unsigned char>(s[1]))))
        {
          const int century_field_val = year_;
          year_ =  (century_field_val * 100) + get_two_digits(&s[0]);
          s += 2;               // this consumes exactly two characters.
        }
    }
  else
    {
      year_ = y2k_window(year_);
    }
#endif

  month_     = get_part(s, 12);
  month_day_ = get_part(s, days_in_month(month_, year_));
  hour_      = get_part(s, 23);
  minute_    = get_part(s, 59);
  second_    = get_part(s, 59);

  update_yearday();
}

// Construct a date as specified in an SCCS file.
sccs_date::sccs_date(const char *date_arg, const char *time)
  : year_(-1), month_(-1), month_day_(-1),
    hour_(-1), minute_(-1), second_(-1),
    yearday_(-1)
{
  std::string date(date_arg);
  int century;

  /* Check for the symtoms of SourceForge bug ID 513800, where
   * the Data General version of Unix puts a four-digit year
   * into the p-file.
   */
  if (date.size() > 4
      && is_digit(date[0])
      && is_digit(date[1])
      && is_digit(date[2])
      && is_digit(date[3])
      && ('/' == date[4]))
  {
      warning("this file has been written by a version of SCCS"
	      " which uses four-digit years, which is contrary to the"
	      " common SCCS file format (though it might have been a "
	      " good idea in the first place)\n");
      century = get_two_digits(date, 0);
      date = std::string(date, 2);
  }
  else
  {
      // this is a normal two-digit date.
      century = 0;              // decide by windowing.
  }

  // Peter Kjellerstedt writes:-
  //
  // This is a gross hack to handle that some old implementation of SCCS
  // has a Y2K bug that results in that dates are written incorrectly as
  // :0/01/01 instead of 00/01/01 (the colon is of course the next
  // character after '9' in the ASCII table). The following should handle
  // this correctly for years up to 2069 (after which the time format
  // used is not valid anyway).
  if (date[0] >= ':' && date[0] <= '@')
    {
      warning("date in SCCS file contains character '%c': "
	      "a version of SCCS which is not Year 2000 compliant "
	      "has probably been used on this file.\n",
	      date[0]);
      /* We're about to subtract 10 from date[0], so make sure that's safe. */
      static_assert(':' > 10, "unsupported character encoding");
      date[0] = static_cast<char>(date[0] - 10);
    }

  // The "1" in the if() is just there to make Emacs align the columns.
  if (1
      && is_digit(date[0]) && is_digit(date[1]) && date[2] == '/'
      && is_digit(date[3]) && is_digit(date[4]) && date[5] == '/'
      && is_digit(date[6]) && is_digit(date[7]) && date[8] == 0

      && is_digit(time[0]) && is_digit(time[1]) && time[2] == ':'
      && is_digit(time[3]) && is_digit(time[4]) && time[5] == ':'
      && is_digit(time[6]) && is_digit(time[7]) && time[8] == '\0')
    {
      year_      = get_two_digits(date, 0);
      month_     = get_two_digits(date, 3);
      month_day_ = get_two_digits(date, 6);

      hour_      = get_two_digits(&time[0]);
      minute_    = get_two_digits(&time[3]);
      second_    = get_two_digits(&time[6]);

      if (century)
      {
          // SourceForge bug ID 513800 - Data General Unix uses 4-digit year
          // in the p-file.
          year_ = century * 100 + year_;
	  ASSERT(year_ >= 1969);
	  ASSERT(year_ <  2069);
      }
      else
      {
	// Year 2000 fix (mandated by X/Open white paper, see above
	// for more details).
	year_ = y2k_window(year_);
      }
      update_yearday();
    }
}

cssc::Failure
sccs_date::printf(FILE *f, char fmt) const
{
  const int yy = year_ % 100;

  switch(fmt)
    {
    case 'D':
      return fprintf_failure(fprintf(f, "%02d/%02d/%02d",
				     yy, month_, month_day_));
    case 'H':
      return fprintf_failure(fprintf(f, "%02d/%02d/%02d",
				     month_, month_day_, yy));

    case 'T':
      return fprintf_failure(fprintf(f, "%02d:%02d:%02d",
				     hour_, minute_, second_));
    }

  int value = 0;
  switch (fmt)
    {
    case 'y':
      value = yy;
      break;

    case 'o':
      value = month_;
      break;

    case 'd':
      value = month_day_;
      break;

    case 'h':
      value = hour_;
      break;

    case 'm':
      value = minute_;
      break;

    case 's':
      value = second_;
      break;

    default:
      ASSERT(!"sccs_date::printf: Invalid format");
      /* This line is reached when ASSERT expands to nothing. */
      return cssc::make_failure_builder_from_errno(EINVAL)
	.diagnose() << "sccs_date::printf: Invalid format letter '"
		    << fmt << "'";
    }
  return fprintf_failure(fprintf(f, "%02d", value));
}

cssc::Failure
sccs_date::print(FILE *f) const
{
  cssc::Failure fail = this->printf(f, 'D');
  fail = Update(fail, fputc_failure(' ', f));
  fail = Update(fail, this->printf(f, 'T'));
  return fail;
}


std::string
sccs_date::as_string() const
{
  char buf[18];
  const int yy = year_ % 100;

  sprintf(buf, "%02d/%02d/%02d %02d:%02d:%02d",
          yy, month_, month_day_,
          hour_, minute_, second_);

  return std::string(buf);
}

sccs_date::sccs_date(int yr, int mth, int day,
                     int hr, int min, int sec)
  : year_(yr), month_(mth), month_day_(day),
    hour_(hr), minute_(min), second_(sec),
    yearday_(-1)
{
  update_yearday();
}

sccs_date::sccs_date()
  : year_(-1), month_(-1), month_day_(-1),
    hour_(-1), minute_(-1), second_(-1), yearday_(-1)
{
}

sccs_date
sccs_date::now()                // static member.
{
  time_t tt;
  constexpr time_t invalid_time = static_cast<time_t>(-1);
  if (time(&tt) == invalid_time)
    {
      errormsg_with_errno("unable to determine current time");
      throw CsscQuitException(2);
    }
  struct tm *ptm = localtime(&tt);
  if (ptm == nullptr)
    {
      errormsg_with_errno("unable to determine local time");
      throw CsscQuitException(2);
    }

  return sccs_date(ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday,
                   ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
}

void
sccs_date::update_yearday()
{
  int m=1, d=1;

  yearday_ = 1;

  while (m < month_)
    yearday_ += days_in_month(m++, year_);

  while (d++ < month_day_)
    yearday_++;
}


int sccs_date::compare(const sccs_date& d) const
{
  ASSERT(valid());
  ASSERT(d.valid());

  int diff;

  if ( (diff = year_ - d.year_) != 0 )
    return diff;
  else if ( (diff = yearday_ - d.yearday_) != 0)
    return diff;

  /* The implementation below can compute an incorrect result when
     comparing dates on a day when there is a clock change.  For
     example, suppose the clock is put pack an hour the first time it
     reaches 02:00 on this day.  Let t1 be a time 3602 seconds into
     this day, and t2 be a time 7201 seconds into this day.  For both,
     hour_=1 and minute_=0 but t1 has second_=2 and t2 has second_=1.
     This function will decide that t1 > t2 while in reality t2 > t1.

     So in theory this implementation is incorrect.  However, since
     the SCCS date format does not include time zone information, we
     cannot receive t2 as input in this case (it would bre represnted
     as the time 01:00::01 and would be understood to represent a time
     3601 seconds into the day).
   */
  if ((diff = hour_ - d.hour_) != 0)
    return diff;
  else if ((diff = minute_ - d.minute_) != 0)
    return diff;
  else
    return second_ - d.second_;
}

bool sccs_date::operator >(sccs_date const & d) const
{
  return compare(d) > 0;
}

bool sccs_date::operator <(sccs_date const &d) const
{
  return compare(d) < 0;
}

bool sccs_date::operator <=(sccs_date const &d) const
{
  return compare(d) <= 0;
}

bool
sccs_date::valid() const
{
  // Allow the seconds field to get as high as 61, since that is what
  // the ANSI C spec for struct tm says, and we have to use a struct
  // tm with localtime().
  return year_ >= 0
    && month_ > 0 && month_ < 13
    && month_day_ > 0 && month_day_ <= days_in_month(month_, year_)
    && hour_ >= 0 && hour_ < 24
    && minute_ >= 0 && minute_ < 60
    && second_ >= 0 && second_ <= 61;
}


/* Local variables: */
/* mode: c++ */
/* End: */
