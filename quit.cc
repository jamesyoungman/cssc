/*
 * quit.cc: Part of GNU CSSC.
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
 * Functions for cleaning up and quitting.
 *
 */

#ifdef __GNUC__
#pragma implementation "quit.h"
#endif

#include "cssc.h"
#include "sysdep.h"
#include "quit.h"
#include "sysnerr.h"
// #include "pipe.h"

#include <stdarg.h>

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: quit.cc,v 1.16 1998/08/13 18:10:56 james Exp $";
#endif

#ifdef CONFIG_BORLANDC
#ifdef __STDC__
extern "C" char * CDECL strlwr(char *);
#endif
#include <dir.h>
#endif /* CONFIG_BORLANDC */

const char *prg_name = NULL;

void
set_prg_name(const char *name) {
#ifdef CONFIG_BORLANDC

	static char file[MAXFILE + MAXEXT];
	char ext[MAXEXT];

	fnsplit(name, NULL, NULL, file, ext);
	strlwr(file);
	strlwr(ext);

	if (strcmp(ext, ".exe") != 0 && strcmp(ext, ".com") != 0) {
		strcat(file, ext);
	}
	prg_name = file;

#else /* CONFIG_BORLANDC */

	prg_name = name;

#endif /* CONFIG_BORLANDC */
}

static void
v_errormsg(const char *fmt, va_list ap)
{
  putc('\n', stderr);
	
  if (prg_name != NULL)
    fprintf(stderr, "%s: ", prg_name);

  vfprintf(stderr, fmt, ap);
}

void errormsg(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  v_errormsg(fmt, ap);
  va_end(ap);
  putc('\n', stderr);
}

void errormsg_with_errno(const char *fmt, ...)
{
  int saved_errno = errno;
  
  va_list ap;
  va_start(ap, fmt);
  v_errormsg(fmt, ap);
  va_end(ap);
  
  errno = saved_errno;
  perror("");
}

static void print_err(int err)
{
#ifndef HAVE_STRERROR
  if (err <= sys_nerr)
    fprintf(stderr, "%d - %s\n", err, sys_errlist[err]);
  else
    fprintf(stderr, "%d - Unknown error\n", err);
#else
  fprintf(stderr, "%d - %s\n", err, strerror(err));
#endif
}

static NORETURN
v_quit(int err, const char *fmt, va_list ap) {
	fflush(stdout);
	cleanup::run_cleanups();

	fflush(stdout);


	if (err == -2) {
	  putc('\n', stderr);
	  usage();
	}
	else
	  {
	    putc('\n', stderr);
	  }
	
	v_errormsg(fmt, ap);

	putc('\n', stderr);

	if (err >= 1) {
	  print_err(err);
	}

	fflush(stderr); // probably not required, stderr is unbuffered.

#ifdef HAVE_ABORT
	if (err == -3) {
		abort();
	}
#endif

#ifdef USE_PIPE
	if (cleanup::in_child()) {
		if (err > 0) {
			_exit(1);
		} else {
			_exit(-err);
		}
	}
#endif

	if (err > 0) {
		exit(1);
	} else {
		exit(-err);
	}
}


NORETURN
quit(int err, const char *fmt, ...) {
	va_list ap;


	va_start(ap, fmt);
	v_quit(err, fmt, ap);
	va_end(ap);
	
	assert(0);		// not reached.
}

// We eventually want to change the code to eliminate most calls
// to quit (because processing should continue with the next input
// file).  We just use this differently-named function to indicate
// the places where a failure odducrs in a constructor, which cannot
// return a failure status.  When they become commonly supported,
// we'll use exceptions.
NORETURN
ctor_quit(int err, const char *fmt, ...) {
	va_list ap;


	va_start(ap, fmt);
	v_quit(err, fmt, ap);
	va_end(ap);
	
	assert(0);		// not reached.
}



NORETURN
nomem() {
	quit(-4, "Out of memory!");
}

NORETURN
assert_failed(const char *file, int line,
	      const char *func,
	      const char *test)
{
  if (func[0])
    {
      quit(-3, "Assertion failed in %s:\nFile: %s  Line: %d: assert(%s)",
	   func, file, line, test);
    }
  else
    {
      quit(-3, "Assertion failed: %s\nFile: %s  Line: %d", test, file, line);
    }
}


class cleanup *cleanup::head = NULL;
int cleanup::running = 0;
int cleanup::all_disabled = 0;
#if HAVE_FORK
int cleanup::in_child_flag = 0;
#endif

cleanup::cleanup() {
	next = head;
	head = this;
}

cleanup::~cleanup() {
	class cleanup *p = head;

	if (p == this) {
		head = next;
		return;
	}

	while (p->next != this) {
		p = p->next;
		ASSERT(p != NULL);
	}

	p->next = next;
}		

void
cleanup::run_cleanups() {
	if (running || all_disabled) {
		return;
	}
	running = 1;

	class cleanup *p = head;
	while (p != NULL) {
		p->do_cleanup();
		p = p->next;
	}

	running = 0;
	all_disabled++;
	return;
}


Cleaner::~Cleaner()
{
  cleanup::run_cleanups();
}

Cleaner::Cleaner()
{
  // nothing to do...
}



/* Local variables: */
/* mode: c++ */
/* End: */
