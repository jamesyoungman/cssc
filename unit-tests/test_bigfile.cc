/*
 * test_bigfile.cc: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 2010 Free Software Foundation, Inc.
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
 * Unit tests which creates a large SCCS file.
 *
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "mylist.h"
#include "mystring.h"
#include "sid.h"
#include "sccsdate.h"
#include "delta.h"
#include "sccsfile.h"
#include "version.h"
#include "my-getopt.h"
#include "fileiter.h"

const char control = '\001';

void
usage ()
{
  fprintf(stderr, "usage: %s [-V] file ...\n", prg_name);
}

static bool
emit_ixg(FILE *fp, char signifier, const mylist<sid>& items)
{
  if (items.length())
    {
      fprintf(fp, "%c%c ", control, signifier);
      for (int i=0; i<items.length(); ++i)
        {
          fprintf(fp, "%s%s", (i ? "," : ""), items[i].as_string().c_str());
        }
      fprintf(fp, "\n");
    }
  return true;
}

static bool
emit_comments_or_mrs(FILE *fp,
                     char signifier,
                     const mylist<mystring> items)
{
  if (items.length() == 0)
    {
      fprintf(fp, "%c%c \n", control, signifier);
    }
  else
    {
      for (int i=0; i<items.length(); ++i)
        {
          fprintf(fp, "%c%c %s\n", control, signifier, items[i].c_str());
        }
    }
}

static bool
emit_delta(FILE *fp,
           unsigned long inserted,
           unsigned long deleted,
           unsigned long unchanged,
           char type,
           sid id,
           const sccs_date& stamp,
           const mystring& user,
           seq_no seq,
           seq_no prev_seq,
           const mylist<sid>& included,
           const mylist<sid>& excluded,
           const mylist<sid>& ignored,
           const mylist<mystring>& comments,
           const mylist<mystring>& mrs)
{
  fprintf(fp,
          "%cs %05lu/%05lu/%05lu\n", control, inserted, deleted, unchanged);
  fprintf(fp,
          "%cd %c %s %s %s %d %d\n",
          control,
          type, id.as_string().c_str(), stamp.as_string().c_str(),
          user.c_str(), seq, prev_seq);
  emit_ixg(fp, 'i', included);
  emit_ixg(fp, 'x', excluded);
  emit_ixg(fp, 'g', ignored);
  emit_comments_or_mrs(fp, 'c', comments);
  if (mrs.length())
    {
      emit_comments_or_mrs(fp, 'm', mrs);
    }
  fprintf(fp, "%ce\n", control);
}


static int
getseq(int release, int level, int branch, int revision,
       int releases,
       int levels_per_release,
       int branches_per_level,
       int revisions_per_branch)
{
  int result = 0;
  int ir, il, ib, is;
  int revisions_per_level = 1 + revisions_per_branch * branches_per_level;
  int revisions_per_release = revisions_per_level * levels_per_release;

  fprintf(stderr, "getseq: input: %d.%d.%d.%d\n", release, level, branch, revision);

  result += revisions_per_release * (release-1);
  result += revisions_per_level   * (level-1);
  result += revisions_per_branch  * branch;
  result += revision;
  result += 1;

  fprintf(stderr, "getseq: output: %d\n", result);
  assert (result >= 0);
  return result;
}


static void
getpred (int *r, int *l, int *b, int *s)
{
  if ((*b))
    {
      if ((*s) > 1)
        {
          --*s;
        }
      else
        {
          *s = 0;
          *b = 0;
        }
    }
  else
    {
      if ((*l) > 1)
        {
          --(*l);
        }
      else
        {
          if ((*r) > 1)
            {
              --(*r);
              (*l) = 1;
            }
          else
            {
              *r = *l = 0;
            }
        }
    }
  if (!(*b) || !(s))
    {
      assert ((*b) == 0);
      assert ((*s) == 0);
    }
}


static bool make_delta(FILE *fp,
                       const sccs_date& current_time,
                       const mystring& username,
                       int release, int level, int branch, int revision,
                       int releases,
                       int levels_per_release,
                       int branches_per_level,
                       int revisions_per_branch)
{
  int prev[4];
  prev[0] = release;
  prev[1] = level;
  prev[2] = branch;
  prev[3] = revision;

  getpred(&prev[0], &prev[1], &prev[2], &prev[3]);

  fprintf(stderr,
          "sid: %d.%d.%d.%d => prev: %d.%d.%d.%d\n",
          release, level, branch, revision,
          prev[0], prev[1], prev[2], prev[3]);

  const sid id(release, level, branch, revision);
  const int this_seq = getseq(release, level, branch, revision,
                              releases, levels_per_release,
                              branches_per_level, revisions_per_branch);
  int prev_seq;
  if (this_seq > 1)
    prev_seq = getseq(prev[0], prev[1], prev[2], prev[3],
                      releases, levels_per_release,
                      branches_per_level, revisions_per_branch);
  else
    prev_seq = 0;

  const mylist<mystring> empty_string_list;
  const mylist<sid> no_sids;
  emit_delta(fp, 0u, 0u, 0u, 'D', id, current_time, username,
             this_seq, prev_seq, no_sids, no_sids, no_sids,
             empty_string_list, empty_string_list);
}

static bool
create_sccs_file(FILE *fp,
                 const mystring& username,
                 int releases,
                 int levels_per_release,
                 int branches_per_level,
                 int revisions_per_branch)
{
  int release, level, branch, revision;
  const sccs_date current_time(sccs_date::now());

  fprintf(fp, "%ch00000\n", control);

  if (revisions_per_branch)
    assert (branches_per_level > 0);
  if (branches_per_level)
    assert (revisions_per_branch > 0);

  // make the branches
  for (release=releases; release>0; --release)
    {
      for (level=levels_per_release; level>0; --level)
        {
          for (branch=branches_per_level; branch>0; --branch)
            {
              for (revision=revisions_per_branch; revision>0; --revision)
                {
                  if (revision && branch)
                    {
                      make_delta(fp, current_time, username,
                                 release, level, branch, revision,
                                 releases, levels_per_release, branches_per_level, revisions_per_branch);
                    }
                }
            }
        }
    }
  // make the trunk.
  revision = branch = 0;
  for (release=releases; release>0; --release)
    {
      for (level=levels_per_release; level>0; --level)
        {
          make_delta(fp, current_time, username,
                     release, level, 0, 0,
                     releases, levels_per_release, branches_per_level, revisions_per_branch);
        }
    }

  fprintf(fp, "%cu\n", control);
  fprintf(fp, "%cU\n", control);
  fprintf(fp, "%ct\n", control);
  fprintf(fp, "%cT\n", control);

  fprintf(fp, "%cI 1\n", control);
  fprintf(fp, "%cE 1\n", control);
}

int
main (int argc, char *argv[])
{
  const mystring username("fred");
  int i, rv, c;

  set_prg_name (argv[0]);
  rv = 0;
  class CSSC_Options opts(argc, argv, "V");
  for (c = opts.next();
       c != CSSC_Options::END_OF_ARGUMENTS;
       c = opts.next())
    {
      switch (c)
        {
        default:
          errormsg("Unsupported option: '%c'", c);
          return 2;

        case 'V':
          version();
          if (2 == argc)
            return 0; // "admin -V" should succeed.
        }
    }
  sccs_file_iterator iter(opts);
  while (iter.next())
    {
      sccs_name &name = iter.get_name();

      int fd = open (name.sfile().c_str(), O_WRONLY|O_CREAT, 0400);
      if (fd >= 0)
        {
          FILE *fp = fdopen(fd, "w");
          create_sccs_file(fp,
                           username,
                           8,           // releases
                           40,          // levels per release
                           10,          // branches per level,
                           10);         // revisions per branch
          fclose (fp);

          // Fix the checksum.
          sccs_file file (name, sccs_file::FIX_CHECKSUM);
          if (!file.update_checksum())
            {
              fprintf (stderr, "Failed to update the checksum of file %s", argv[i]);
              rv = 1;
            }
        }
    }
  return rv;
}
