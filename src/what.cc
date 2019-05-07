/*
 * what.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 1999, 2001, 2007, 2009, 2010, 2011, 2014,
 *  2019 Free Software Foundation, Inc.
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
 * Prints out SCCS identification keywords marked with "@(#)" in files.
 *
 */

#define NO_COMMON_HEADERS

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "defaults.h"
#include "my-getopt.h"
#include "cssc.h"
#include "version.h"

#ifdef CONFIG_WHAT_USE_STDIO

/* Inline fuctions for reading files with stdio. */

typedef FILE *XFILE;

const XFILE XOPEN_FAILURE = 0;

void usage(void);

inline XFILE
xopen(const char *name) {
#ifdef CONFIG_OPEN_SCCS_FILES_IN_BINARY_MODE
	return fopen(name, "rb");
#else
	return fopen(name, "r");
#endif
}

inline int
xclose(XFILE f) {
	return fclose_failed(fclose(f));
}

inline ssize_t
xread(XFILE f, char *buf, int len) {
	int ret = fread(buf, len, 1, f);
	if (ret != len && ferror(f)) {
		return -1;
	}
	return len;
}

inline int
xgetc(XFILE f) {
	return getc(f);
}

#else /* CONFIG_WHAT_USE_STDIO */

/* Inline functions for reading files with Unix style I/O */

#include "sysdep.h"

typedef int XFILE;

const XFILE XOPEN_FAILURE = -1;

inline XFILE
xopen(const char *name) {
#ifdef CONFIG_OPEN_SCCS_FILES_IN_BINARY_MODE
	return open(name, O_RDONLY | O_BINARY);
#else
	return open(name, O_RDONLY);
#endif
}

inline int
xclose(XFILE f) {
	return close(f);
}

inline ssize_t
xread(XFILE f, char *buf, size_t len)
{
  return read(f, buf, len);
}

static inline void
fail()
{
  exit(1);
}

inline int
xgetc(XFILE f)
{
  char c;
  int ret = read(f, &c, 1);
  if (ret == -1)
    {
      fprintf(stderr, "Read error (%s)\n", strerror(errno));
      fail();
      return EOF;
    }
  else if (ret == 0)
    {
      return EOF;
    }
  else
    {
      return c;
    }
}

#endif /* CONFIG_WHAT_USE_STDIO */

static const char *what_prg_name = "what";

void
usage(void)
{
  fprintf(stderr, "usage: %s [-sV] file ...\n", what_prg_name);
}



/* Print what's found after a "@(#)" in a file */

inline char *
print_what(char *s, char *end, XFILE f) {
	putchar('\t');

	while (s < end) {
		char c = *s;
		switch (c) {
		case '"':
		case '>':
		case '\n':
#if CONFIG_EOL_CHARACTER != '\n'
		case CONFIG_EOL_CHARACTER:
#endif
		case '\\':
		case '\0':
			return s;
		}
		putchar(c);
		s++;
	}

	int c = xgetc(f);
	while (c != EOF) {
		switch (c) {
		case '"':
		case '>':
		case '\n':
#if CONFIG_EOL_CHARACTER != '\n'
		case CONFIG_EOL_CHARACTER:
#endif
		case '\\':
		case '\0':
		  /* Note that only one test case currently covers this line
		   * of the code.   Test w22 of whatbasic.sh exercises this,
		   * but nothing else does.
		   */
		  return end;
		}
		putchar(c);
		c = xgetc(f);
	}

	return 0;
}

#ifndef CONFIG_WHAT_BUFFER_SIZE
#define CONFIG_WHAT_BUFFER_SIZE (16*1024)
#endif

int
main(int argc, char **argv)
{
  int one_match = 0;
  int matchcount = 0;

  if (argc > 0)
    what_prg_name = argv[0];

  check_env_vars();

  int c;
  class CSSC_Options opts(argc, argv, "r!snV", 1);
  for (c = opts.next(); c != CSSC_Options::END_OF_ARGUMENTS; c = opts.next())
    {
      switch (c)
	{
	case 's':
	  one_match = 1;
	  break;

	case 'V':
	  version();
	}
    }

  int arg;
  for (arg = opts.get_index(); arg < argc; arg++)
    {
      XFILE f = xopen(argv[arg]);
      if (f == XOPEN_FAILURE)
	{
	  fprintf(stderr, "%s: Can't open file (%s)\n",
		  argv[arg], strerror(errno));
	  fail();
	}

      printf("%s:\n", argv[arg]);

      static char buf[CONFIG_WHAT_BUFFER_SIZE + 3];
      buf[0] = buf[1] = buf[2] = '\0';

      ssize_t read_len = xread(f, buf + 3, CONFIG_WHAT_BUFFER_SIZE);
      while (read_len > 0)
	{
	  char *at = (char *) memchr(buf, '@', static_cast<size_t>(read_len));
	  int done = 0;
	  char *end = buf + read_len;
	  while (at)
	    {
	    if (at[1] == '(' && at[2] == '#' && at[3] == ')')
	      {
		at = print_what(at+4, end + 3, f);
		++matchcount;

		putchar('\n');
		if (0 == at || one_match)
		  {
		    done = 1;
		    break;
		  }
	      }
	    at++;
	    if (at >= end)
	      {
		break;
	      }
	    at = (char *) memchr(at, '@', end - at);
	  }
	  if (done)
	    {
	      break;
	    }
	  buf[0] = end[0];
	  buf[1] = end[1];
	  buf[2] = end[2];
	  read_len = xread(f, buf + 3, CONFIG_WHAT_BUFFER_SIZE);
	}
      if (read_len == -1)
	{
	  fprintf(stderr, "%s: Read error (%s)\n",
		  argv[arg], strerror(errno));
	  fail();
	}
      xclose(f);
    }
  if (matchcount)
    return 0;
  else
    return 1;
}

/* Local variables: */
/* mode: c++ */
/* End: */
