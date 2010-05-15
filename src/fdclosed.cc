/*
 * fdclosed.cc: Part of GNU CSSC.
 *
 *    Copyright (C) 1998,2000,2007 Free Software Foundation, Inc.
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
 */

/* This class exists to ensure that the standard file descriptors are
 * not closed.  That would mean that we could open an ordinary file
 * and then corrupt it since we don't realise that output to stdout or
 * stderr will go to it.
 *
 *
 * If any of the file descriptors 0, 1, 2 are not open, attach them
 * to /dev/null so that we don't fopen() a file, get a low numbered
 * file descriptor, and accidentally corrupt our file with a printf()
 * to stdout or stderr, or trying to read from stdin (hence changing
 * the file pointer on our file too).
 */

#include "cssc.h"
#include "sysdep.h"
#include "defaults.h"		// CONFIG_NULL_FILENAME

#include <unistd.h>
#include <stdio.h>		/* perror() */


class SafeFdCheck
{
public:
  SafeFdCheck();
};


/* We want to emit an error message.  If stderr has been closed, this
 * is difficult.  We just send the output to the closed file
 * descriptor (using perror()).
 *
 * We do this becauase (1) there isn't really a better option, and
 * (2) because the output will still show up in the strace(8) output
 * if the user really needs to track down the problem.
 *
 * If you have an idea for a better way of implementing it, please
 * feel free to do so; but remember, it needs to work when we just
 * failed to open() /dev/null.  So opening another file (e.g. /dev/log
 * with syslog) isn't going to work.
 */

SafeFdCheck::SafeFdCheck()
{
  int i, fd;

  for (i=0; i<3; ++i)
    {
      fd = open(CONFIG_NULL_FILENAME, O_RDONLY, 0);
      if (fd < 0)
	{
	  perror(CONFIG_NULL_FILENAME);
	  _exit(1);
	}
      else if (fd > 2)
	{
	  close(fd);
	  return;
	}
    }
}


/* Instantiate the class so that the constructor is called to do the check.
 */
static SafeFdCheck PleaseDoIt;

/* Local variables: */
/* mode: c++ */
/* End: */
