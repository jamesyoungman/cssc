/*
 * file.cc: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1998, 1999, 2001, 2007, 2008, 2009, 2010, 2011,
 *  2014, 2019 Free Software Foundation, Inc.
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
 * System dependent routines for accessing files.
 *
 */
#include "config.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <stdlib.h>

#include "cssc.h"		/* for CONFIG_CAN_HARD_LINK_AN_OPEN_FILE */
#include "cssc-assert.h"
#include "failure.h"
#include "failure_or.h"
#include "sysdep.h"
#include "file.h"
#include "quit.h"
#include "ioerr.h"
#include "defaults.h"
#include "dirent-safer.h"
#include "privs.h"
#include "stringify.h"

using cssc::Failure;
using cssc::FailureOr;


/* Redirects stdout to a "null" file (eg. /dev/null). */
cssc::Failure
stdout_to_null()
{
  if (nullptr == freopen(CONFIG_NULL_FILENAME, "w", stdout))
    {
      return cssc::make_failure_builder_from_errno(errno)
	.diagnose()
	<< "can't redirect stdout to " << CONFIG_NULL_FILENAME;
    }
  else
    {
      return cssc::Failure::Ok();
    }
}


/* Returns true if stdin is not a file. */

bool
stdin_is_a_tty() {
        return isatty(0);
}


/* Opens a stream to write to the "null" file (eg. /dev/null). */

cssc::FailureOr<FILE *>
open_null()
{
  FILE *f = fopen(CONFIG_NULL_FILENAME, "w");
  if (nullptr == f)
    {
      return cssc::make_failure_builder_from_errno(errno)
	<< "failed to optn " << CONFIG_NULL_FILENAME;
    }
  return f;
}


/* Returns true if the file exists and is readable. */

bool
is_readable(const char *name)
{
  return access(name, R_OK) != -1;
}

static bool
get_mode_bits(const char *filename, mode_t mask, mode_t *result)
{
  struct stat st;
  if (0 != stat(filename, &st))
    return false;

  *result = (st.st_mode & mask);
  return true;
}

cssc::FailureOr<bool>
get_open_file_xbits (FILE *f)
{
  const int fd = fileno(f);
  if (fd < 0)
    return cssc::make_failure_from_errno(errno);

  struct stat st;
  if (0 != fstat(fd, &st))
    return cssc::make_failure_from_errno(errno);

  return st.st_mode & 0111;
}




/* Determine if a given file is "writable".  If we
 * are root, we can write to a mode 000 file, but
 * we deem files of that sort not writable for these
 * purposes -- we in fact only care about this for
 * the safeguards in "get -e" and "get".  This avoids
 * overwriting a file which we might be editing.
 *
 * The previous implementation used to actually use
 * access/eaccess, but those return 0 for root for
 * files that we choose to not actually count as
 * writeable.
 *
 * Note that is_readable still uses access.  I don't
 * know (now) why the disparity.  But the regression
 * test suite doesn't include setuid execution, so it
 * would likely not notice a difference in these
 * semantics.
 */
static bool
is_writable(const char *filename, int /* as_real_user = 1 */ )
{
  mode_t bits;
  if (!get_mode_bits(filename, 0222, &bits))
    return 0;			// cannot tell
  return bits ? 1 : 0;
}

/* Returns true if the file exists. */

int
file_exists(const char *name) {
        return access(name, 0) != -1;
}


#ifdef CONFIG_UIDS

/* This function is documented in the subsection "USER" in the
 * CSSC manual.
 */
const char *
get_user_name()
{
  struct passwd *p = getpwuid(getuid());
  if (nullptr == p)
    {
      fatal_quit(-1, "UID %s not found in password file.",
		 cssc::stringify(getuid()).c_str());
    }
  return p->pw_name;
}

bool
user_is_group_member(gid_t gid)
{
  return gid == getegid();
}

#else /* CONFIG_UIDS */

/* This function is documented in the subsection "USER" in the
 * CSSC manual.
 */
const char *
get_user_name()
{
  const char *s = getenv("USER");
  if (s)
    return s;
  else
    return "unknown";
}

bool
user_is_group_member(int)
{
  return 0;
}
#endif /* CONFIG_UIDS */


/* From the Linux manual page for open(2):-
 *
 * O_EXCL When used with O_CREAT, if the file already exists it is an
 *      error and the open will fail.  O_EXCL is broken on NFS file
 *      systems, programs which rely on it for performing locking
 *      tasks will contain a race condition.  The solution for
 *      performing atomic file locking using a lockfile is to create
 *      a unique file on the same fs (e.g., incorporating hostname
 *      and pid), use link(2) to make a link to the lockfile and use
 *      stat(2) on the unique file to check if its link count has
 *      increased to 2.  Do not use the return value of the link()
 *      call.
 */


/* WARNING: If you use fstat() on the fd to get the link count, the
 * wrong value is returned for Linux 2.0.34 and glibc-2.0.7, the
 * second time we perform the fstat().  Therefore we use stat(2)
 * rather than fstat(2).
 */
static FailureOr<nlink_t> get_nlinks(const char *name)
{
  struct stat st;

  if (0 == stat(name, &st))
    {
      return st.st_nlink;
    }
  else
    {
      return cssc::make_failure_from_errno(errno);
    }
}

static void
maybe_wait_a_bit(long attempt, const char *lockfile)
{
    unsigned int waitfor = 0u;

    if (attempt > 4)
    {
        /* Make sure that it's unlikely for two instances to be in sync. */
        if ((attempt & 1) == (getpid() & 1) )
        {
            /* Once we have been waiting for a while, make each wait
             * longer in order to reduce the load on the system.  In
             * this case it is unlikely that we will ever get the lock -
             * perhaps the other process got killed with SIGKILL or
             * something like that - but we can't just go ahead since
             * that's too dangerous.
             *
             * Therefore we hang around until someone investigates and
             * either kills us or deletes the lock file.  Meanwhile we
             * hope that our output messages are not being logged to a
             * file that's filled the disk.
             */
            if (attempt < 10)
                waitfor = 1u;
            else
                waitfor = 10u;
        }
    }

    if (waitfor)
    {
      // TODO: the cast below is likely wrong if pid_t is unsigned;
      // switch errormsg (or a parallel feature) to using ostream
      // inserters so that we no longer have to match a format string
      // with the types of the arguments.
        errormsg("Sleeping for %d second%s "
                 "while waiting for lock on %s; my PID is %ld",
                 waitfor,
                 ( (waitfor == 1) ? "" : "s"),
                 lockfile,
		 static_cast<long int>(getpid()));
        sleep (waitfor);
    }
}


static FailureOr<int> atomic_nfs_create(const std::string& path, int flags, int perms)
{
  auto fallback = [&path, flags, perms]() -> cssc::FailureOr<int>
    {
      /* Our fallback option is to assume that the file system
       * supports O_EXCL if it does not support link(2).
       *
       * Assuming that O_EXCL works is less safe than not asuuming it,
       * since NFS v2 clients have to "fake" O_EXCL.
       */
      int fd = open(path.c_str(), flags, perms);
      if (fd < 0)
	return cssc::make_failure_from_errno(errno);
      return fd;
    };

  std::string dirname, basename;
  split_filename(path, dirname, basename);

  /* Rely (slightly) on only 11 characters of filename. */
  for (long attempt=0; attempt < 10000; ++attempt)
    {
      /* form the name of a lock file. */
      char buf[32];
      sprintf(buf, "nfslck%ld", attempt);
      const std::string lockname = dirname + std::string(buf);
      const char *lockstr = lockname.c_str();

      errno = 0;
      int fd = open(lockstr, flags, perms);
      if (fd >= 0)
        {
	  cssc::FailureOr<nlink_t> links = get_nlinks(lockstr);
	  if (!links.ok())
	    {
	      // stat not supported on this file system or the file
	      // was deleted.
	      return fallback();
	    }
          if (*links == 1)
            {
              int link_errno = 0;
              errno = 0;
              if (-1 == link(lockstr, path.c_str()))
                link_errno = errno;

              /* ignore other responses */
	      cssc::FailureOr<nlink_t> links_again = get_nlinks(lockstr);
              if (links_again.ok() && (2 == *links_again))
                {
                  unlink(lockstr);
                  return fd;    /* success! */
                }
              else              /* link(2) failed. */
                {
		  close(fd);
		  unlink(lockstr);

		  /* A VirtualBox shared folder (Solaris guest, Linux host,
		     ext3 as the underlying file system) returns ENOSYS when
		     we attempt to use link(2). */
                  if (EPERM == link_errno || ENOSYS == link_errno)
                    {
                      /* containing file system does not support hard links. */
		      return fallback();
                    }
                  else
		    {
                      /* The z.* file exists; wait a bit. */
                      maybe_wait_a_bit(attempt, path.c_str());
		    }
                }
            }
          close(fd);
          unlink(lockstr);
        }
      else                      /* open() failed. */
        {
          switch (errno)
            {
            case EEXIST:
              /* someone else got that lock first; they may in fact not
               * be trying to lock the same s-file (but instead another
               * s-file in the same directory)
               *
               * Try again.  Sleep first if we're not doing well,
               * but try to avoid pathalogical cases...
               */
              maybe_wait_a_bit(attempt, path.c_str());
              break;

            default:            /* hard failure. */
              return fallback();
            }
        }
    }
  return -1;
}


/* CYGWIN seems to be unable to create a file for writing, with mode
 * 0444, so this code resets the mode after we have closed the g-file.
 */
cssc::Failure set_file_mode(const std::string &gname, bool writable, bool executable)
{
  const char *name = gname.c_str();
  struct stat statbuf;
  int fd = open(name, O_RDONLY);
  if (fd < 0)
    {
      return cssc::make_failure_builder_from_errno(errno)
	.diagnose() << name << ": cannot open file in order to change its mode";
    }
  else
    {
      if (0 == fstat(fd, &statbuf))
	{
	  mode_t mode = statbuf.st_mode & 0666;
	  if (writable)
	    mode |= 0200;
	  else
	    mode &= (~0200);

	  if (executable)
	    {
	      // A SCO extension.
	      mode |= 0111;
	    }

	  if (0 == fchmod(fd, mode))
	    {
	      close(fd);
	      return cssc::Failure::Ok();
	    }
	  else
	    {
	      const int saved_errno = errno;
	      close(fd);
	      return cssc::make_failure_builder_from_errno(saved_errno)
		.diagnose() << name << ": cannot set mode of file to "
			    << std::setfill('0') << std::oct << std::setw(4) << mode;
	    }
	}
      else
	{
	  return cssc::make_failure_builder_from_errno(errno)
	    .diagnose() << "cannot stat file " << name;
	}
    }
}

/* Set the mode of the g-file.  We need to give up privs to do this,
 * since the user as whom we are running setuid does not necessarily
 * have search privs on the current directory, or privs to chmod
 * the real user's files.  However, we need to do this (for example,
 * for get -k).  This is SourceForge bug 451519.
 */
cssc::Failure set_gfile_writable(const std::string& gname, bool writable, bool executable)
{
  TempPrivDrop guard();
  return set_file_mode(gname, writable, executable);
}


/* When doing "get -e" we need to remove any existing read-only g-file.
 * To do so we have to restore real user privileges, because the current
 * directory may not be accessible to the setuid user.  Equally, we
 * should prevent the caller from deleting arbitrary read-only files
 * that they would not ordinarily be able to delete.  Note though that
 * CSSC is not intended for setuid or setgid operation.
 * This is SourceForge bug number 481707.
 */

cssc::Failure unlink_gfile_if_present(const char *gfile_name)
{
  TempPrivDrop guard();
  if (file_exists(gfile_name))
    {
      if (unlink(gfile_name) < 0)
        {
          return cssc::make_failure_builder_from_errno(errno)
	    .diagnose() << "cannot unlink the file " << gfile_name;
        }
    }
  return cssc::Failure::Ok();

}

/* unlink_file_as_real_user
 *
 * Unlinks the specified file as the real user.
 * The caller is responsible for issuing any error message,
 */
cssc::Failure unlink_file_as_real_user(const char *gfile_name)
{
  TempPrivDrop guard();
  if (unlink(gfile_name) < 0)
    {
      // The caller is responsible for issuing any error message,
      return cssc::make_failure_from_errno(errno);
    }
  return cssc::Failure::Ok();
}

static bool is_overwrite_ok(const std::string& name, int mode)
{
#ifdef CONFIG_USE_ARCHIVE_BIT
  if (file_exists(name.c_str()) && test_archive_bit(name.c_str()))
    {
      errormsg("%s: File exists and its archive attribute is set.",
	       name.c_str());
      return false;
    }
#else
  if (is_writable(name.c_str(), mode & CREATE_AS_REAL_USER))
    {
      errormsg("%s: File exists and is writable.", name.c_str());
      return false;
    }
#endif
  return true;
}


static int convert_createfile_mode_to_open_mode(int mode)
{
  int flags = O_CREAT;

  if (mode & CREATE_FOR_UPDATE)
    {
      flags |= O_RDWR;
    }
  else
    {
      flags |= O_WRONLY;
    }

  if (mode & CREATE_EXCLUSIVE)
    {
      flags |= O_EXCL;
    }
  return flags;
}

static int convert_createfile_mode_to_perms(int mode)
{
  int perms = 0644;
  if (mode & CREATE_READ_ONLY)
    {
      perms = 0444;
    }
  if (mode & CREATE_EXECUTABLE)
    {
      perms |= 0111;
    }
  return perms;
}


/* returns a file descriptor open to a newly created file. */
static FailureOr<int>
createfile(const std::string& name, int mode) {
  const int flags = convert_createfile_mode_to_open_mode(mode);
  const int perms = convert_createfile_mode_to_perms(mode);

  if (!(mode & CREATE_EXCLUSIVE))
    {
      if ((mode & CREATE_FOR_GET) && !is_overwrite_ok(name, mode))
	{
	  return cssc::make_failure(cssc::errorcode::DeclineToOverwriteOutputFile);
	}
      else
	{
	  auto unlinked = unlink_gfile_if_present(name.c_str());
	  if (!unlinked.ok())
	    return unlinked;
	}
    }

  TempPrivDrop drop(mode & CREATE_AS_REAL_USER);
  if (CONFIG_CAN_HARD_LINK_AN_OPEN_FILE && (mode & CREATE_NFS_ATOMIC) )
    {
      cssc::FailureOr<int> fofd = atomic_nfs_create(name.c_str(), flags, perms);
      if (!fofd.ok())
	return fofd.fail();
      return *fofd;
    }
  else
    {
      const int fd = open(name.c_str(), flags, perms);
      if (fd < 0)
	return cssc::make_failure_from_errno(errno);
      return fd;
    }
}


/* Returns a file stream open to a newly created file. */

cssc::FailureOr<FILE *>
fcreate(const std::string& name, int mode) {
  cssc::FailureOr<int> fofd = createfile(name.c_str(), mode);
  if (!fofd.ok())
    {
      return fofd.fail();
    }
  const int fd = *fofd;
  FILE* result = fdopen(fd,(mode & CREATE_FOR_UPDATE) ? "w+" : "w");
  if (result == NULL)
    {
      return cssc::make_failure_builder_from_errno(errno)
	<< "unable to associate a mode \"" << mode << "\" FILE* with a file "
	<< "descriptor open on file " << name;
    }
  return result;
}


FILE *fopen_as_real_user(const char *s, const char *mode)
{
  TempPrivDrop guard();
  return fopen(s, mode);
}



static cssc::Failure
put_pid(FILE *f)
{
  auto val = static_cast<unsigned long int>(getpid());
  if (fprintf_failed(fprintf(f, "%lu", val)))
    return cssc::make_failure_from_errno(errno);
  return cssc::Failure::Ok();
}


static cssc::Failure
do_lock(FILE *f)                // process-aware version
{
  return put_pid(f);
}


file_lock::file_lock(const std::string& zname)
  : lock_state_(),		// empty optional.
    name_(zname)
{
  ASSERT(name_ == zname);
  cssc::FailureOr<FILE*> fof =
    fcreate(zname, CREATE_READ_ONLY | CREATE_EXCLUSIVE | CREATE_NFS_ATOMIC);
  if (!fof.ok())
    {
      // XXX: do not assume errno has been preserved.
      if (errno == EEXIST)
        {
	  ASSERT(lock_state_.has_value());
	  ASSERT(lock_state_.value().ok());
          return;
        }
      lock_state_ =
	(cssc::FailureBuilder(fof.fail())
	 .diagnose() << "can't create lock file " <<  zname);
      ctor_fail_nomsg(1);
    }

  FILE *f = *fof;
  cssc::Failure done = do_lock(f);
  if (fclose_failed(fclose(f)))
    {
      done = cssc::Update(done,
			  cssc::make_failure_builder_from_errno(errno)
			  .diagnose() << "failed to close " << zname);
    }
  if (!done.ok())
    {
      remove(zname.c_str());
      ctor_fail_nomsg(1);
    }
  lock_state_ = done;
  return;
}

file_lock::~file_lock() {
  if (lock_state_.has_value()) {
    lock_state_.reset();
    unlink(name_.c_str());
  }
}



cssc::FailureOr<bool>
is_directory(const char *name)
{
  errno = 0;
  DIR *p = opendir_safer(name);
  if (p)
    {
      closedir(p);
      return true;
    }
  else if (ENOTDIR == errno)
    {
      return false;
    }
  return cssc::make_failure_builder_from_errno(errno)
    << "unable to determine whether " << name
    << " is a directory";
}



/* Local variables: */
/* mode: c++ */
/* End: */
