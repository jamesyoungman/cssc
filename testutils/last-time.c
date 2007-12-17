/* last-time.c: Part of GNU CSSC.
 * 
 *    Copyright (C) 1998, Free Software Foundation, Inc. 
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
 *
 * This program is not installed as part of CSSC.  It's just desinged
 * to help you figure out the year-2038 date at which your C library
 * gives out.
 *
 * No attempt has been made to make this program use the
 * Autoconf-generated configuration information.
 */



#include <stdio.h>
#include <time.h>


static int* 
next_field(struct tm *ptm,
	   int *current)
{
  int *next;
  
  if (current == &ptm->tm_year)
    next = &ptm->tm_mon;
  else if (current == &ptm->tm_mon)
    next = &ptm->tm_mday;
  else if (current == &ptm->tm_mday)
    next = &ptm->tm_hour;
  else if (current == &ptm->tm_hour)
    next = &ptm->tm_min;
  else if (current == &ptm->tm_min)
    next = &ptm->tm_sec;
  else
    next = NULL;

  return next;
}

int main()
{
  struct tm tm_time;
  struct tm *ptm;
  time_t now;
  time_t then;
  long maxiters = 2000L;
  int *pfield;
  int old_field_val;

  tm_time.tm_year = 97;		/* 1997 */
  tm_time.tm_mon = 0;		/* counts from zero for some reason. */
  tm_time.tm_mday = 1;
  tm_time.tm_hour = 0;
  tm_time.tm_min = 0;
  tm_time.tm_sec = 0;
  tm_time.tm_isdst = -1;
  
  ptm = &tm_time;
  pfield = &ptm->tm_year;

  old_field_val = *pfield;
  
  while (maxiters--)
    {
      now = mktime(ptm);
      if ( (time_t)-1 == then )
	{
	  perror("mktime error");
	  exit(1);
	}
      else if (difftime(now, then) < 0.0)
	{
	  *pfield = old_field_val;
	  pfield = next_field(ptm, pfield);
	  if (NULL == pfield)
	    {
	      /* Print the "final" time */
	      printf("%s", ctime(&then));
	      exit(0);		/* done. */
	    }
	}
      else
	{
	  then = now;
	}
      
      old_field_val = *pfield;
      ++*pfield;
    }
  printf("The time representation doesn't seem to"
	 " break very easily; is this a 64-bit system (or more)?\n");
  exit(0);			/* this can happen too. */
}

