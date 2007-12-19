/*
 * quit.cc: Part of GNU CSSC.
 * 
 *    Copyright (C) 1997,1998,1999,2001,2007 Free Software Foundation, Inc. 
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
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Functions for cleaning up and quitting.
 *
 */

#if defined(__GNUC__) \
&& !(defined(__APPLE__) && __GNUC__ == 3 && __GNUC_MINOR__ == 1)
#pragma implementation "quit.h"
#endif

#include "cssc.h"
#include "sysdep.h"
#include "quit.h"
#include "sysnerr.h"
#include "except.h"
#include "err_no.h"

// #include "pipe.h"

#include <stdarg.h>

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: quit.cc,v 1.34 2007/12/19 00:21:14 jay Exp $";
#endif

const char *prg_name = NULL;

void
set_prg_name(const char *name) {
        prg_name = name;
}

static void
v_errormsg(const char *fmt, va_list ap)
{
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

void warning (const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "%s: warning: ", prg_name);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  putc('\n', stderr);
}

void v_errormsg_with_errno(const char *fmt, va_list ap)
{
  int saved_errno = errno;
  
  v_errormsg(fmt, ap);
  errno = saved_errno;
  perror(" ");
}

void errormsg_with_errno(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  v_errormsg_with_errno(fmt, ap);
  va_end(ap);
  putc('\n', stderr);
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

	// We used to call usage() is err was -2, but
	// nobody ever actually used that.
	
	putc('\n', stderr);
        if (fmt)
          {
            v_errormsg(fmt, ap);
            putc('\n', stderr);
          }
        
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

#ifdef HAVE_EXCEPTIONS  
        if (err < 0)
          err = -err;
        throw CsscQuitException(err);
#else
        if (err > 0) {
                exit(1);
        } else {
                exit(-err);
        }
#endif  
}

// We eventually want to change the code to eliminate most calls
// to quit (because processing should continue with the next input
// file).  We just use this differently-named function to indicate
// the places where a failure odducrs in a constructor, which cannot
// return a failure status.  When they become commonly supported,
// we'll use exceptions.
NORETURN
ctor_fail(int err, const char *fmt, ...) {
        va_list ap;


        va_start(ap, fmt);
#ifdef HAVE_EXCEPTIONS
        if (fmt)
          {
            v_errormsg(fmt, ap);
            putc('\n', stderr);
          }
        throw CsscContstructorFailedException(err);
#else
        v_quit(err, fmt, ap);
#endif
        /*NOTREACHED*/
        va_end(ap);
        ASSERT(0);              // not reached.
}


#ifndef HAVE_EXCEPTIONS
/* We have to pass a va_list to v_quit, to we have to have 
 * a varargs function to do it...  
 */

static NORETURN  ctor_fail_nomsg_helper(int err, ...)
{
    va_list ap;
    va_start(ap, err);
    v_quit(err, NULL, ap);
    /*NOTREACHED*/
    va_end(ap);
}
#endif

NORETURN ctor_fail_nomsg(int err)  
{
#ifdef HAVE_EXCEPTIONS
    throw CsscContstructorFailedException(err);
#else
    ctor_fail_nomsg_helper(err);
    /*NOTREACHED*/
#endif
}



NORETURN
s_corrupt_quit(const char *fmt, ...) {
        va_list ap;


        va_start(ap, fmt);
#ifdef HAVE_EXCEPTIONS  
        v_errormsg(fmt, ap);
        putc('\n', stderr);
        throw CsscSfileCorruptException();
#else
        v_quit(-1, fmt, ap);
#endif
        /*NOTREACHED*/
        va_end(ap);
        ASSERT(0);              // not reached.
}

NORETURN
s_missing_quit(const char *fmt, ...) {
        va_list ap;


#ifdef HAVE_EXCEPTIONS  
        va_start(ap, fmt);
        v_errormsg_with_errno(fmt, ap);
        va_end(ap);
        
        putc('\n', stderr);
        throw CsscSfileMissingException();
#else
        va_start(ap, fmt);
        v_quit(-1, NULL, ap);
        va_end(ap);
#endif
        /*NOTREACHED*/
        ASSERT(0);              // not reached.
}


/* s_unrecognised_feature_quit is usually called by 
 * sccs_file::saw_unknown_feature().
 */
NORETURN
s_unrecognised_feature_quit(const char *fmt, va_list ap)
{
  if (prg_name != NULL)
    fprintf(stderr, "%s: Warning: unknown feature: ", prg_name);
  
#ifdef HAVE_EXCEPTIONS  
  vfprintf(stderr, fmt, ap);
  putc('\n', stderr);
  throw CsscUnrecognisedFeatureException();
#else
  v_quit(-1, fmt, ap);
#endif
}

void
v_unknown_feature_warning(const char *fmt, va_list ap)
{
  if (prg_name != NULL)
    fprintf(stderr, "%s: Warning: unknown feature: ", prg_name);

  vfprintf(stderr, fmt, ap);
}


NORETURN
p_corrupt_quit(const char *fmt, ...) {
        va_list ap;


        va_start(ap, fmt);
#ifdef HAVE_EXCEPTIONS  
        v_errormsg(fmt, ap);
        putc('\n', stderr);
        throw CsscPfileCorruptException();
#else
        v_quit(-1, fmt, ap);
#endif
        /*NOTREACHED*/
        va_end(ap);
        ASSERT(0);              // not reached.
}

// fatal_quit() is ALWAYS immediately fatal.
NORETURN
fatal_quit(int err, const char *fmt, ...) {
        va_list ap;

        va_start(ap, fmt);
        v_quit(err, fmt, ap);
        /*NOTREACHED*/
        va_end(ap);
        ASSERT(0);              // not reached.
}



NORETURN
nomem() {
        fatal_quit(-4, "Out of memory!");
}

NORETURN
assert_failed(const char *file, int line,
              const char *func,
              const char *test)
{
  if (func[0])
    {
      fatal_quit(-3, "Assertion failed in %s:\nFile: %s  Line: %d: assert(%s)",
                 func, file, line, test);
    }
  else
    {
      fatal_quit(-3, "Assertion failed: %s\nFile: %s  Line: %d",
                 test, file, line);
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

cleanup::~cleanup()
{
  if (head == NULL)
    {
      // SourceForge bug # 816679; cleanup::cleanup() 
      // can be called before there are any entries on the 
      // list.
      return; // nothing to do.
    }
  else
    {
      class cleanup *p = head;
      
      if (p == this) 
	{
	  head = next;
	  return;
        }
      
      while (p->next != this) 
	{
	  p = p->next;
	  ASSERT(p != NULL);
        }
      
      if (p != NULL) // 'if' fixes SourceForge bug # 816679
	{
	  p->next = next;
	}
    }
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
  quit_on_fatal_signals();
}



/* Local variables: */
/* mode: c++ */
/* End: */
