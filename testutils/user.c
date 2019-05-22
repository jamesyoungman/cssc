/*
 * user.c: Part of GNU CSSC.
 *
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2007,
 * 2008, 2009, 2010, 2001, 2014, 2019 Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Program for getting the user's login name.
 */
#include <config.h>

#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "progname.h"
#include "gettext.h"

const char usage_str[] = "usage: \"user name\" or \"user group\"\n";


static int duplicate_group(gid_t g, const gid_t *vec, int len)
{
  while (len--)
    {
      if (*vec == g)
        return 1;
    }
  return 0;
}


/* do_groups
 *
 * Emits a list of groups associated with the current
 * process.  The effectve group ID is also returned,
 * and this may be a suplicate entry.
 */
static gid_t *get_group_list(int *ngroups)
{
  int len;
  gid_t *grouplist;

  len = getgroups(0, NULL);

  if (len < 0)
    {
      perror("getgroups");
      return NULL;
    }
  else
    {
      grouplist = malloc((1+len) * sizeof(*grouplist));
      if (grouplist)
        {
          /* We don't know if the effectve group ID is in
           * the list returned by grouplist, so find out
           * and return a list with it included, but only once
           */
          gid_t egid = getegid();
          if (getgroups(len, &grouplist[1]))
            {
              if (duplicate_group(egid, grouplist+1, len))
                {
                  *ngroups = len;
                  return &grouplist[1];
                }
              else
                {
                  grouplist[0] = egid;
                  *ngroups = len+1;
                  return &grouplist[0];
                }
            }
          else
            {
              perror("getgroups");
              return NULL;
            }
        }
      else
        {
              perror("malloc");
              return NULL;
        }
    }
}


static void do_groups()
{
  int ngroups;
  const gid_t *list = get_group_list(&ngroups);

  if (list)
    {
      int i;

      for (i=0; i<ngroups; ++i)
        {
          fprintf(stdout, "%ld\n", (long) list[i]);
        }
    }
  else
    {
      /* fallback. */
      fprintf(stdout, "%ld\n", (long) getegid());
    }
}


static int compare_groups(const void *pv1, const void *pv2)
{
  const gid_t *p1 = (const gid_t*) pv1;
  const gid_t *p2 = (const gid_t*) pv2;

  if (*p1 < *p2)
    return -1;
  else if (*p1 > *p2)
    return 1;
  else
    return 0;
}


/* Find and print the Id of a group of which we are not a member.
 * This group will (most likely) not have a name
 */
static gid_t foreign_group(void)
{
  int ngroups, i;
  gid_t *list = get_group_list(&ngroups);

  qsort(list, ngroups, sizeof(*list), compare_groups);

  for (i=1; i<ngroups; ++i)
    {
      /* nextval is a gid_t value 1 greater than the last gid we saw */
      const gid_t nextval = 1+list[i-1];

      if (nextval < list[i] )
        {
          /* we have a gap. */
          return nextval;
        }
    }

  /* We can't just add 1 to the value of the last entry in the list
   * because of the possibility of overflow.
   */
  if (ngroups)
    {
      if (list[0] > 0)
        return 0;
      else
        return 1+list[ngroups-1];
    }
  else /* not a member of any groups? */
    {
      gid_t last_resort = getegid();
      if (last_resort > 0)
        return 0;
      else
        return 1+last_resort;
    }
}


int main(int argc, char *argv[])
{
  set_program_name (argv[0]);
  if (NULL == setlocale(LC_ALL, ""))
    {
      /* If we can't set the locale as the user wishes,
       * emit an error message and continue.   The error
       * message will of course be in the "C" locale.
       */
      perror("Error setting locale");
    }
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  if (2 == argc)
    {
      if (0 == strcmp(argv[1], "name"))
        {
          struct passwd *p;
          const char *pn = "unknown";
          p = getpwuid(getuid());
          if (p)
            pn = p->pw_name;

          fprintf(stdout, "%s\n", pn);
          return 0;
        }
      else if (0 == strcmp(argv[1], "groups"))
        {
          do_groups();
          return 0;
        }
      else if (0 == strcmp(argv[1], "foreigngroup"))
        {
          fprintf(stdout, "%ld\n", (long)foreign_group());
          return 0;
        }
      else if (0 == strcmp(argv[1], "group"))
        {
          fprintf(stdout, "%ld\n", (long)getgid());
          return 0;
        }
      else
        {
          fprintf(stderr, "%s", usage_str);
          return 1;
        }
    }
  else
    {
      fprintf(stderr, "%s", usage_str);
      return 1;
    }
}
