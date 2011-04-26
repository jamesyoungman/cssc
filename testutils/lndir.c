
/* lndir.c; part of GNU CSSC.
 *
 *    Copyright (C) 1997, Free Software Foundation, Inc.
 *
 * This file is derived from xc/config/util/lndir.c in the X11R6 distribution.
 * It's been changed to make use of GNU Autoconf rather than xmkmf (imake)
 * for portability.
 *   -- James Youngman <jay@gnu.org>
 */


/* $XConsortium: lndir.c,v 1.13 94/04/17 20:10:42 rws Exp $ */
/* Create shadow link tree (after X11R4 script of the same name)
   Mark Reinhold (mbr@lcs.mit.edu)/3 January 1990 */

/*
Copyright (c) 1990,  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/

/* From the original /bin/sh script:

  Used to create a copy of the a directory tree that has links for all
  non-directories (except those named RCS, SCCS or CVS.adm).  If you are
  building the distribution on more than one machine, you should use
  this technique.

  If your master sources are located in /usr/local/src/X and you would like
  your link tree to be in /usr/local/src/new-X, do the following:

        %  mkdir /usr/local/src/new-X
        %  cd /usr/local/src/new-X
        %  lndir ../X
*/
#include <config.h>

#ifndef HAVE_SYMLINK
#error I need to be patched to support either hard links or copying.
#endif

#ifndef HAVE_READLINK
#error I need to be patched to support either hard links or copying.
#endif


#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>

#include "dirent-safer.h"

#ifndef MAXPATHLEN
#define MAXPATHLEN 2048
#endif

int silent;

char *rcurdir;
char *curdir;

static void vmsg (const char *, int, va_list);

void
quit (int code, int err_num, char * fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vmsg (fmt, err_num, args);
    va_end(args);
    exit (code);
}

static void
vmsg (const char *fmt, int errnum, va_list ap)
{
  assert (fmt && fmt[0]);
  if (curdir) {
    fprintf (stderr, "%s:\n", curdir);
    curdir = 0;
  }
  vfprintf (stderr, fmt, ap);
  if (errnum)
    {
      fprintf (stderr, ": %s", strerror (errnum));
    }
  putc ('\n', stderr);
}

void
mperror (int err_num, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vmsg (fmt, err_num, args);
  va_end(args);
}

int equivalent(char *lname, const char *rname)
{
    char *s;

    if (!strcmp(lname, rname))
        return 1;
    for (s = lname; *s && (s = strchr(s, '/')); s++) {
        while (s[1] == '/')
            strcpy(s+1, s+2);
    }
    return !strcmp(lname, rname);
}

static int is_dir(const struct stat *p)
{
  return S_ISDIR(p->st_mode);
}

static int ignored(const char *n)
{
  const char *histdirs[] = {"SCCS","RCS","CVS.adm","CVS", (const char*)0};
  const char **p;
  for (p=histdirs; *p; ++p)
    {
      if (0 == strcmp(*p, n))
        return 1;
    }
  return 0;
}

static int
is_dot_or_dotdot (const char *p)
{
  if (p[0] == '.') {
    if (p[1] == 0) {
      return 1;			/* . */
    } else if (p[1] == '.' && p[2] == 0) {
      return 1;			/* .. */
    }
  }
  return 0;
}


/* Recursively create symbolic links from the current directory to the "from"
   directory.  Assumes that files described by fs and ts are directories. */

int
dodir (const char *fn, /* name of "from" directory, either absolute or relative to cwd */
       const struct stat *fs, /* stats for the "from" directory */
       const struct stat *ts, /* stats for the cwd */
       int rel)
{
    DIR *df;
    struct dirent *dp;
    char buf[MAXPATHLEN + 1], *p;
    char symbuf[MAXPATHLEN + 1];
    struct stat sb, sc;
    int symlen;
    char *ocurdir;

    if ((fs->st_dev == ts->st_dev) && (fs->st_ino == ts->st_ino))
      {
        mperror (errno,
		 "%s: From and to directories are identical, "
		 "hence no work to do!", fn);
        return 0;               /* nothing to do! */
      }

    if (rel)
        strcpy (buf, "../");
    else
        buf[0] = '\0';
    strcat (buf, fn);

    if (!(df = opendir (buf))) {
        mperror (errno, "%s: Cannot opendir", buf);
        return 1;
    }

    p = buf + strlen (buf);
    *p++ = '/';

    while ( (struct dirent*)0 != (dp = readdir (df)) ) {
        const size_t namlen = strlen(dp->d_name);
	if (namlen) {
	  if (dp->d_name[namlen - 1] == '~')
            continue;
	}
	if (is_dot_or_dotdot (dp->d_name)) {
	  continue;		/* ignore. */
	}


        strcpy (p, dp->d_name);

        if (1) {
            if (stat (buf, &sb) < 0) {
	        mperror (errno, "failed to stat %s", buf);
                continue;
            }


            if (is_dir(&sb))
            {
                /* directory */
                if (is_dot_or_dotdot (dp->d_name) || ignored (dp->d_name))
                    continue;

                ocurdir = rcurdir;
                rcurdir = buf;
                curdir = silent ? buf : (char *)0;
                if (!silent)
                    printf ("making links in %s:\n", buf);
                if ((stat (dp->d_name, &sc) < 0) && (errno == ENOENT)) {
                    if (mkdir (dp->d_name, 0777) < 0 ||
                        stat (dp->d_name, &sc) < 0) {
		        mperror (errno, "failed to stat %s", dp->d_name);
                        curdir = rcurdir = ocurdir;
                        continue;
                    }
                }
                if (readlink (dp->d_name, symbuf, sizeof(symbuf) - 1) >= 0) {
		    mperror (0, "%s: is a link instead of a directory",
			     dp->d_name);
                    curdir = rcurdir = ocurdir;
                    continue;
                }
                if (chdir (dp->d_name) < 0) {
		    mperror (errno,
			     "failed to change directory into %s", dp->d_name);
                    curdir = rcurdir = ocurdir;
                    continue;
                }
                dodir (buf, &sb, &sc, (buf[0] != '/'));
                if (chdir ("..") < 0)
		  quit (1, errno, "..");
                curdir = rcurdir = ocurdir;
                continue;
            }
        }

        /* non-directory */
        symlen = readlink (dp->d_name, symbuf, sizeof(symbuf) - 1);
        if (symlen >= 0) {
            symbuf[symlen] = '\0';
            if (!equivalent (symbuf, buf))
                mperror (0, "%s: %s", dp->d_name, symbuf);
        } else if (symlink (buf, dp->d_name) < 0)
            mperror (errno,
		     "failed to create a symbolic link %s pointing to %s",
		     dp->d_name, buf);
    }

    closedir (df);
    return 0;
}

int
main (int ac, char *av[])
{
    char *fn, *tn;
    struct stat fs, ts;

    silent = 0;
    if (ac > 1)
      {
        if (!strcmp(av[1], "--silent")) /* GNU-style long options. */
          silent = 1;
        else if (!strcmp(av[1], "-silent")) /* X11R4 compatibility. */
          silent = 1;
      }

    if (ac < silent + 2 || ac > silent + 3)
        quit (1, 0, "usage: %s [-silent] fromdir [todir]", av[0]);

    fn = av[silent + 1];
    if (ac == silent + 3)
        tn = av[silent + 2];
    else
        tn = ".";

    /* to directory */
    if (stat (tn, &ts) < 0)
        quit (1, errno, "%s", tn);

    if (!is_dir(&ts))
        quit (2, 0, "%s: Not a directory", tn);

    if (chdir (tn) < 0)
        quit (1, errno, "%s", tn);

    /* from directory */
    if (stat (fn, &fs) < 0)
        quit (1, errno, "%s", fn);

    if (!is_dir(&fs))
        quit (2, 0, "%s: Not a directory", fn);

    return dodir (fn, &fs, &ts, 0);
}
