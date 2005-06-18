/*
 * file.cc: Part of GNU CSSC.
 * 
 *    Copyright (C) 1997,1998,1999,2001 Free Software Foundation, Inc. 
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * System dependent routines for accessing files.
 *
 */

#if defined(__GNUC__) \
&& !(defined(__APPLE__) && __GNUC__ == 3 && __GNUC_MINOR__ == 1)
#pragma implementation "filelock.h"
#endif

#include "cssc.h"
#include "sysdep.h"
#include "err_no.h"
#include "file.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <stdio.h>

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: file.cc,v 1.38 2005/06/18 23:16:25 james_youngman Exp $";
#endif

#ifdef CONFIG_UIDS

#ifdef CONFIG_DECLARE_STAT
extern "C" int CDECL stat(const char *, struct stat *);
#endif



/* Tests the accessability of file, but unlike access() uses the
   effective uid and gid, not the real uid and gid. */

static int
eaccess(const char *name, mode_t perm) {
        struct stat st;

        if (stat(name, &st) == -1) {
                return -1;
        }

        perm &= 7;
        if (perm == 0) {
                return 0;
        }

        if (geteuid() == st.st_uid) {
                st.st_mode >>= 6;
        } else if (getegid() == st.st_gid) {
                st.st_mode >>= 3;
        }

        if ((st.st_mode & perm) == perm) {
                return 0;
        }

        return -1;
}

#else /* CONFIG_UIDS */

inline int
eaccess(const char *name, int perm) {
        return access(name, perm);
}

#endif /* CONFIG_UIDS */


/* Redirects stdout to a "null" file (eg. /dev/null). */
                
bool
stdout_to_null()
{
  if (NULL == freopen(CONFIG_NULL_FILENAME, "w", stdout))
    {
      errormsg_with_errno("Can't redirect stdout to "
                          CONFIG_NULL_FILENAME );
      return false;
    }
  else
    {
      return true;
    }
}


/* Redirects to stdout to stderr. */

FILE *
stdout_to_stderr() {
        fflush(stdout);
        fflush(stderr);

        int out = dup(1);
        if (out == -1)
          {
            errormsg_with_errno("dup(1) failed");
            return NULL;
          }

        if (close(1) == -1 || dup(2) != 1)
          {
            errormsg_with_errno("Can't redirect stdout to stderr");
            return NULL;
          }

        FILE *f = fdopen(out, "w");
        if (f == NULL)
          {
            errormsg_with_errno("fdopen failed");
            return NULL;
          }
        return f;
}

/* Returns true if stdin is not a file. */

int
stdin_is_a_tty() {
        return isatty(0);
}


/* Opens a stream to write to the "null" file (eg. /dev/null). */

FILE *
open_null()
{
  FILE *f = fopen(CONFIG_NULL_FILENAME, "w");
  if (NULL == f)
    perror(CONFIG_NULL_FILENAME);
  return f;
}


/* Returns true if the file exists and is readable. */

int
is_readable(const char *name) {
        return access(name, 04) != -1;
}

/* Determine if a given file is "writable".  If we
 * are root, we can write to a mode 000 file, but
 * we deem files of that sort not writable for these
 * purposes -- we in fact only care about this for 
 * the safeguards in "get -e" and "get".  This avoids
 * overwriting a file which we might be editing.
 */
int
is_writable(const char *filename, int /* as_real_user = 1 */ )
{
  struct stat st;
  if (0 != stat(filename, &st))
    {
      return 0;                 // can't stat it so can't read it, probably.
    }
  else
    {
      if (st.st_mode & 0222)
        return 1;               // at least one of the write bits is set.
      else
        return 0;               // no write bits set.
    }
  
}

/* Returns true if the file exists and is writable. */

inline int
old_is_writable(const char *name, int as_real_user = 1) {
        if (as_real_user) {
                return access(name,  (mode_t) 02) != -1;
        } else {
                return eaccess(name, (mode_t) 02) != -1;
        }
}


/* Returns true if the file exists. */

int
file_exists(const char *name) {
        return access(name, 0) != -1;
}


                

#ifdef CONFIG_UIDS

/* A flag to indicate whether or not the programme is an privileged
   (effective UID != real UID) or unprivileged (effective UID == real
   UID). */

static int unprivileged = 0;

#ifdef SAVED_IDS_OK

static uid_t old_euid;

// Set-user-id is saved.  TODO: What about setuid-root binaries?
void
give_up_privileges() {
        if (unprivileged++ == 0) {
                old_euid = geteuid();
                if (setuid(getuid()) == -1) {
                        fatal_quit(errno, "setuid(%d) failed", getuid());
                }
                ASSERT(getuid() == geteuid());
        }
}

void
restore_privileges() {
        if (--unprivileged == 0) {
                if (setuid(old_euid) == -1) { 
                        fatal_quit(errno, "setuid(%d) failed", old_euid);
                }
                ASSERT(geteuid() == old_euid);
        }
        ASSERT(unprivileged >= 0);
}

#elif defined(HAVE_SETREUID)

// POSIX saved IDs not provided or not always provided; use
// setreuid() instead.

static uid_t old_ruid, old_euid;

void
give_up_privileges() {
        if (unprivileged++ == 0) {
                old_ruid = getuid();
                old_euid = geteuid();

                if (setreuid(old_euid, old_ruid) == -1) {
                        fatal_quit(errno, "setreuid(%d, %d) failed.",
                                   old_euid, old_ruid);
                }
                ASSERT(geteuid() == old_ruid);
        }
}

void
restore_privileges() {
        if (--unprivileged == 0) {
                if (setreuid(old_ruid, old_euid) == -1) {
                        fatal_quit(errno, "setreuid(%d, %d) failed.", 
                                   old_ruid, old_euid);
                }
                ASSERT(geteuid() == old_euid);
        }
        ASSERT(unprivileged >= 0);
}


#else /* defined(HAVE_SETREUID) */

// Processes do not have a saved set-user-id,
// and setreuid() is not available.
void
give_up_privileges() {
        ++unprivileged;
        if (geteuid() != getuid()) {
                fatal_quit(-1, "Set UID not supported.");
        }
}

void
restore_privileges() {
        --unprivileged;
        ASSERT(unprivileged >= 0);
}

#endif /* defined(HAVE_SETREUID) */

// inline int
// open_as_real_user(const char *name, int mode, int perm) {
//         give_up_privileges();
//         int fd = open(name, mode, perm);
//         restore_privileges();
//         return fd;
// }

#ifdef CONFIG_DECLARE_GETPWUID
extern "C" struct passwd * CDECL getpwuid(int uid);
#endif

const char *
get_user_name()
{
  struct passwd *p = getpwuid(getuid());
  if (0 == p)
    {
      fatal_quit(-1, "UID %d not found in password file.", getuid());
    }
  return p->pw_name;
}

int
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

int
user_is_group_member(int) {
        return 0;
}


void
give_up_privileges()
{
  // Dummy implementation for systems without Unix-like UIDs.
  unprivileged++;
}

void
restore_privileges()
{
  // Dummy implementation for systems without Unix-like UIDs.
  --unprivileged;
}



#endif /* CONFIG_UIDS */

/* From the Linux manual page for open(2):-
 *
 * O_EXCL When used with O_CREAT, if the file already exists it is an
 *        error and the open will fail.  O_EXCL is broken on NFS file
 *        systems, programs which rely on it for performing locking
 *        tasks will contain a race condition.  The solution for
 *        performing atomic file locking using a lockfile is to create
 *        a unique file on the same fs (e.g., incorporating hostname
 *        and pid), use link(2) to make a link to the lockfile and use
 *        stat(2) on the unique file to check if its link count has
 *        increased to 2.  Do not use the return value of the link()
 *        call.
 */


/* WARNING: If you use fstat() on the fd to get the link count, the
 * wrong value is returned for Linux 2.0.34 and glibc-2.0.7, the
 * second time we perform the fstat().  Therefore we use stat(2)
 * rather than fstat(2).
 */
static long get_nlinks(const char *name)
{
  struct stat st;

  if (0 == stat(name, &st))
    {
      return (long)st.st_nlink;
    }
  else
    {
      return -1L;
    }
}

static void maybe_wait_a_bit(int attempt, const char *lockfile)
{
    int waitfor = 0;
    
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
                waitfor = 1;
            else
                waitfor = 10;
        }
    }

    if (waitfor)
    {
        errormsg("Sleeping for %d second%s "
                 "while waiting for lock on %s; my PID is %ld\n",
                 waitfor,
                 ( (waitfor == 1) ? "" : "s"),
                 lockfile,
                 (long int) getpid() );
        sleep(waitfor);
    }
}


static int atomic_nfs_create(const mystring& path, int flags, int perms)
{
  mystring dirname, basename;
  char buf[32];
  const char *pstr = path.c_str();
  
  split_filename(path, dirname, basename);

  /* Rely (slightly) on only 11 characters of filename. */
  for (long attempt=0; attempt < 10000; ++attempt)
    {
      /* form the name of a lock file. */
      sprintf(buf, "nfslck%ld", attempt);
      const mystring lockname = dirname + mystring(buf);
      const char *lockstr = lockname.c_str();

      errno = 0;
      int fd = open(lockstr, flags, perms);
      if (fd >= 0)
        {
          if (1 == get_nlinks(lockstr))
            {
              int link_errno = 0;
              errno = 0;
              if (-1 == link(lockstr, pstr))
                link_errno = errno;

              /* ignore other responses */
              
              if (2 == get_nlinks(lockstr))
                {
                  unlink(lockstr); 
                  return fd;    /* success! */
                }
              else              /* link(2) failed. */
                {
                  if (EPERM == link_errno)
                    {
                      /* containing filesystem does not support hard links. */
                      close(fd);
                      unlink(lockstr);

                      /* assume that the filesystem supports O_EXCL if it does
                       * not supprort link(2).
                       */
                      return open(pstr, flags, perms);
                    }
                  else
                  {
                      /* The z.* file exists; wait a bit. */
                      maybe_wait_a_bit(attempt, pstr);
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
              maybe_wait_a_bit(attempt, pstr);
              break;
              
            default:            /* hard failure. */
              /* fall back on the less-safe method, which will
               * probably still fail
               */
              return open(pstr, flags, perms);
            }
        }
    }
  return -1;
}


/* CYGWIN seems to be unable to create a file for writing, with mode
 * 0444, so this code resets the mode after we have closed the g-file. 
 */
bool set_file_mode(const mystring &gname, bool writable, bool executable)
{
  const char *name = gname.c_str();
  struct stat statbuf;
  int fd = open(name, O_RDONLY);
  
  if (fd < 0)
    {
      errormsg_with_errno("%s: cannot open file in order to change its mode",
			  name);
      return false;
    }
  else 
    {
      if (0 == fstat(fd, &statbuf))
	{
	  int mode = statbuf.st_mode & 0666;
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
	      return true;
	    }
	  else
	    {
	      errormsg_with_errno("%s: cannot set mode of file to 0%o",
				  name, mode);
	      close(fd);
	      return false;
	    }
	}
      else
	{
	  errormsg_with_errno("%s: cannot stat file", name);
	}
    }
}

/* Set the mode of the g-file.  We need to give up provs to do this, 
 * since the user as whom we are running setuid does not necessarily
 * have search privs on the current directory, or privs to chmod
 * the real user's files.  However, we need to do this (for example, 
 * for get -k).  This is SourceForge bug 451519.
 */
bool set_gfile_writable(const mystring& gname, bool writable, bool executable)
{
  give_up_privileges();
  bool rv = set_file_mode(gname, writable, executable);
  restore_privileges();
  return rv;
}


/* When doing "get -e" we need to remove any existing read-only g-file. 
 * To do so we have to restore real user privileges, because the current 
 * directory may not be accessible to the setuid user.  Equally, we
 * should prevent the caller from deleting arbitrary read-only files 
 * that they would not ordinarily be able to delete.  Note though that 
 * CSSC is not intended for setuid or setgid operation. 
 * This is SourceForge bug number 481707.
 */

bool unlink_gfile_if_present(const char *gfile_name)
{
  give_up_privileges();
  bool rv = true;
  
  if (file_exists(gfile_name))
    {
      if (unlink(gfile_name) < 0)
        {
          errormsg_with_errno("Cannot unlink the file %s", gfile_name);
          rv = false;
        }
    }
  restore_privileges();
  
  return rv;
}

bool unlink_file_if_present(const char *gfile_name)
{
  /* We must also do the existence test as the real user so was 
   * can't delegate the job to unlink_file_as_real_user().
   */
  give_up_privileges();
  bool rv = true;
  
  if (file_exists(gfile_name))
    {
      if (remove(gfile_name) < 0)
        {
	  // The remove() funtion will already have issued an error message.
          rv = false;
        }
    }
  restore_privileges();
  
  return rv;
}

/* unlink_file_as_real_user
 *
 * Unlinks the specified file as the real user.
 * The caller is responsible for issuing any error message,
 */
bool unlink_file_as_real_user(const char *gfile_name)
{
  give_up_privileges();
  bool rv = true;
  if (unlink(gfile_name) < 0)
    {
      // The caller is responsible for issuing any error message,
      rv = false;
    }
  restore_privileges();
  
  return rv;
}


/* returns a file descriptor open to a newly created file. */

int
create(mystring name, int mode) {
        int flags = O_CREAT;

        if (mode & CREATE_FOR_UPDATE) {
                flags |= O_RDWR;
        } else {
                flags |= O_WRONLY;
        }

        if (mode & CREATE_EXCLUSIVE) {
                flags |= O_EXCL;
#ifdef CONFIG_USE_ARCHIVE_BIT
        } else if ((mode & CREATE_FOR_GET)
                   && file_exists(name.c_str())
                   && test_archive_bit(name.c_str())) {
                errormsg("%s: File exists and its archive attribute is set.",
                     name.c_str());
                return -1;
#else
        } else if ((mode & CREATE_FOR_GET)
                   && is_writable(name.c_str(), mode & CREATE_AS_REAL_USER)) {
                errormsg("%s: File exists and is writable.", name.c_str());
                errno = 0;
                return -1;
#endif
        } else if (!unlink_gfile_if_present(name.c_str())) {
                return -1;
        }

#ifdef CONFIG_MSDOS_FILES
        int perms = 0666;
#else
        int perms = 0644;
        if (mode & CREATE_READ_ONLY) {
                perms = 0444;
        }
#endif 

	if (mode & CREATE_EXECUTABLE)
	  {
	    // A SCO extension.
	    perms |= 0111;
	  }
	  
        int fd;

        if (mode & CREATE_AS_REAL_USER)
          {
            give_up_privileges();
          } 
        
#ifdef CONFIG_SHARE_LOCKING
        if (mode & CREATE_WRITE_LOCK)
          {
            fd = sopen(name.c_str(), flags, SH_DENYWR, perms);
          }
        else
          {
            /* These systems don't support link(2)... */
            fd = open(name.c_str(), flags, perms);
          }
#else     
        if (CONFIG_CAN_HARD_LINK_AN_OPEN_FILE && (mode & CREATE_NFS_ATOMIC) )
          fd = atomic_nfs_create(name.c_str(), flags, perms);
        else
          fd = open(name.c_str(), flags, perms);
#endif

        
        if (mode & CREATE_AS_REAL_USER)
          {
            restore_privileges();
          } 
        return fd;
}


/* Returns a file stream open to a newly created file. */

FILE *
fcreate(mystring name, int mode) {
        int fd = create(name.c_str(), mode);
        if (fd == -1) {
                return NULL;
        }
        if (mode & CREATE_FOR_UPDATE) {
                return fdopen(fd, "w+");
        }
        return fdopen(fd, "w");
}


FILE *fopen_as_real_user(const char *s, const char *mode)
{
  FILE *fp = NULL;
  
  give_up_privileges();
  fp = fopen(s, mode);
  restore_privileges();
  return fp;
}



#ifdef CONFIG_SHARE_LOCKING

// this code now in dosfile.cc.

#elif defined(CONFIG_PID_LOCKING) || defined(CONFIG_DUMB_LOCKING)

#if defined(CONFIG_PID_LOCKING)

static int
put_pid(FILE *f)
{
  return fprintf(f, "%lu", (unsigned long int)getpid());
}

#endif


#ifdef CONFIG_DUMB_LOCKING
static int
do_lock(FILE *f)                // dumb version
{
  return fprintf_failed(fprintf(f, "%s\n", get_user_name()))
}
#else
static int
do_lock(FILE *f)                // process-aware version
{
  return put_pid(f) < 0;
}
#endif

file_lock::file_lock(mystring zname): locked(0), name(zname)
{
  ASSERT(name == zname);
#if 0
  fprintf(stderr, "Lock file is \"%s\"\n", zname.c_str());
#endif  
  FILE *f = fcreate(zname,
                    CREATE_READ_ONLY | CREATE_EXCLUSIVE | CREATE_NFS_ATOMIC);
  if (0 == f)
    {
      if (errno == EEXIST)
        {
          return;
        }
      errormsg_with_errno("%s: Can't create lock file", zname.c_str());
      ctor_fail_nomsg(1);
    }

  if (do_lock(f) != 0 || fclose_failed(fclose(f)))
    {
      remove(zname.c_str());
      errormsg_with_errno("%s: Write error", zname.c_str());
      ctor_fail_nomsg(1);
    }
  
  locked = 1;
  return;
}
  
  file_lock::~file_lock() {
        if (locked) {
                locked = 0;
                unlink(name.c_str());
        }
}

#endif /* defined(CONFIG_PID_LOCKING) */




/* Local variables: */
/* mode: c++ */
/* End: */
