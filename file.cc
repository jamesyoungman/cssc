/*
 * file.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * System dependent routines for accessing files.
 *
 */

#ifdef __GNUC__
#pragma implementation "filelock.h"
#endif

#include "mysc.h"
#include "sysdep.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>

#ifdef CONFIG_SCCS_IDS
static const char sccs_id[] = "@(#) MySC file.c 1.1 93/11/09 17:17:53";
#endif

#ifdef CONFIG_UIDS

#ifdef CONFIG_DECLARE_STAT
extern "C" int CDECL stat(const char *, struct stat *);
#endif


/* Tests the accessability of file, but unlike access() uses the
   effective uid and gid, not the real uid and gid. */

static int
eaccess(const char *name, int perm) {
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
		
void
stdout_to_null()
{
  if (NULL == freopen(CONFIG_NULL_FILENAME, "w", stdout))
    {
      quit(errno, "Can't redirect stdout to "
	   CONFIG_NULL_FILENAME ".");
    }
}


/* Redirects to stdout to stderr. */

FILE *
stdout_to_stderr() {
	fflush(stdout);
	fflush(stderr);

	int out = dup(1);
	if (out == -1) {
		quit(errno, "dup(1) failed.");
	}

	if (close(1) == -1 || dup(2) != 1) {
		quit(errno, "Can't redirect stdout to stderr.");
	}

	FILE *f = fdopen(out, "w");
	if (f == NULL) {
		quit(errno, "fdopen failed.");
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
open_null() {
	FILE *f = fopen(CONFIG_NULL_FILENAME, "w");
	if (f == NULL) {
		quit(errno, CONFIG_NULL_FILENAME ": Can't open for writing.");
	}
	return f;
}


/* Returns true if the file exists and is readable. */

int
is_readable(const char *name) {
	return access(name, 04) != -1;
}


/* Returns true if the file exists and is writable. */

inline int
is_writable(const char *name, int as_real_user = 1) {
	if (as_real_user) {
		return access(name, 02) != -1;
	} else {
		return eaccess(name, 02) != -1;
	}
}


/* Returns true if the file exists. */

int
file_exists(const char *name) {
	return access(name, 0) != -1;
}


#if defined(CONFIG_SYNC_BEFORE_REOPEN) || defined(CONFIG_SHARE_LOCKING)

#ifdef CONFIG_NO_FSYNC
#include "fsync.cc"
#endif

#if !defined(fileno) && defined(__BORLANDC__)
#define fileno(f) (f->fd)
#endif

int
ffsync(FILE *f) {
	if (fsync(fileno(f)) == -1) {
		return EOF;
	}
	return 0;
}

#endif /* defined(CONFIG_SYNC_BEFORE_REOPEN) || ... */


#ifdef CONFIG_USE_ARCHIVE_BIT

#ifdef CONFIG_NO__CHMOD

#include "_chmod.cc"

#ifndef FA_ARCH
#define FA_ARCH 0x20
#endif

#endif /* CONFIG_NO__CHMOD */


/* Clears the archive file attribute bit of a file. */

void
clear_archive_bit(const char *name) {
	int attribs = _chmod(name, 0);
	if (attribs == -1 || _chmod(name, 1, attribs & ~FA_ARCH) == -1) {
		quit(errno, "%s: Can't clear archive file attribute.", name);
	}
}


/* Returns true if the archive file attribute bit of a file is set. */

inline int
test_archive_bit(const char *name) {
	int attribs = _chmod(name, 0);
	if (attribs == -1) {
		quit(errno, "%s: Can't test archive file attribute.", name);
	}
	return (attribs & FA_ARCH) == FA_ARCH;
}

#endif /* CONFIG_USE_ARCHIVE_BIT */
		

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
			quit(errno, "setuid(%d) failed", getuid());
		}
		assert(getuid() == geteuid());
	}
}

void
restore_privileges() {
	if (--unprivileged == 0) {
	  // XXX TODO: shouldn't this be setuid(old_euid) ?
		if (setuid(old_euid) == -1) { 
			quit(errno, "setuid(%d) failed", old_euid);
		}
		assert(geteuid() == old_euid);
	}
	assert(unprivileged >= 0);
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
			quit(errno, "setreuid(%d, %d) failed.",
			     old_euid, old_ruid);
		}
		assert(geteuid() == old_ruid);
	}
}

void
restore_privileges() {
	if (--unprivileged == 0) {
		if (setreuid(old_ruid, old_euid) == -1) {
			quit(errno, "setreuid(%d, %d) failed.", 
			     old_ruid, old_euid);
		}
		assert(geteuid() == old_euid);
	}
	assert(unprivileged >= 0);
}


#else /* defined(HAVE_SETREUID) */

// Processes do not have a saved set-user-id,
// and setreuid() is not available.
void
give_up_privileges() {
	++unprivileged;
	if (geteuid() != getuid()) {
		quit(-1, "Set UID not supported.");
	}
}

void
restore_privileges() {
	--unprivileged;
	assert(unprivileged >= 0);
}

#endif /* defined(HAVE_SETREUID) */

inline int
open_as_real_user(const char *name, int mode, int perm) {
	give_up_privileges();
	int fd = open(name, mode, perm);
	restore_privileges();
	return fd;
}

#ifdef CONFIG_DECLARE_GETPWUID
extern "C" struct passwd * CDECL getpwuid(int uid);
#endif

#ifdef CONFIG_DECLARE_GETLOGIN
extern "C" char *getlogin(void);
#endif

const char *
get_user_name() {
	static mystring name = getlogin();
	if (name != NULL) {
		return name;
	}
	struct passwd *p = getpwuid(getuid());
	if (p == NULL) {
		quit(-1, "UID %d not found in password file.", getuid());
	}
	name = p->pw_name;
	endpwent();
	return name;
}

int
user_is_group_member(int gid) {
	return gid == getegid();
}

#else /* CONFIG_UIDS */

const char *
get_user_name() {
	const char *s = getenv("USER");
	if (s != NULL) {
		return s;
	}
	return "unknown";
}

int
user_is_group_member(int) {
	return 0;
}

#endif /* CONFIG_UIDS */


/* Returns a file descriptor open to a newly created file. */

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
		   && file_exists(name) && test_archive_bit(name)) {
		quit(-1, "%s: File exists and its archive attribute is set.",
		     (const char *) name);
#else
	} else if ((mode & CREATE_FOR_GET)
		   && is_writable(name, mode & CREATE_AS_REAL_USER)) {
		quit(-1, "%s: File exists and is writable.",
		     (const char *) name);
#endif
	} else if (file_exists(name) && unlink(name) == -1) {
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

	int fd;

#ifdef CONFIG_UIDS
	if (mode & CREATE_AS_REAL_USER) {
		fd = open_as_real_user(name, flags, perms);
	} else 
#endif
#ifdef CONFIG_SHARE_LOCKING
	if (mode & CREATE_WRITE_LOCK) {
		fd = sopen(name, flags, SH_DENYWR, perms);
	} else
#endif
	{
		fd = open(name, flags, perms);
	}

	return fd;
}


/* Returns a file stream open to a newly created file. */

FILE *
fcreate(mystring name, int mode) {
	int fd = create(name, mode);
	if (fd == -1) {
		return NULL;
	}
	if (mode & CREATE_FOR_UPDATE) {
		return fdopen(fd, "w+");
	}
	return fdopen(fd, "w");
}

#ifdef CONFIG_SHARE_LOCKING

file_lock::file_lock(mystring zname): locked(0), name(zname) {
	f = fcreate(zname, CREATE_WRITE_LOCK | CREATE_EXCLUSIVE);
	if (f == NULL) {
		struct DOSERROR err;

		if (errno == EEXIST
		    || (dosexterr(&err) == EACCES
			&& err.de_class == 2 /* temporary situation */)) {
			return;
		}
		quit(errno, "%s: Can't create lock file.", 
		     (const char *) zname);
	}
	if (fprintf(f, "%s\n", get_user_name()) == EOF
	    || fflush(f) == EOF || ffsync(f) == EOF) {
		quit(errno, "%s: Write error.", (const char *) zname);
	}

	locked = 1;
	return;
}

file_lock::~file_lock() {
	if (locked) {
		locked = 0;
		fclose(f);
		unlink(name);
	}
}

#elif defined(CONFIG_PID_LOCKING) || defined(CONFIG_DUMB_LOCKING)

file_lock::file_lock(mystring zname): locked(0), name(zname) {
	FILE *f = fcreate(zname, CREATE_READ_ONLY | CREATE_EXCLUSIVE);
	if (f == NULL) {
		if (errno == EEXIST) {
			return;
		}
		quit(errno, "%s: Can't create lock file.", 
		     (const char *) zname);
	}

#ifdef CONFIG_DUMB_LOCKING
	if (fprintf(f, "%s\n", get_user_name()) == EOF
#else
	if (putw(getpid(), f) == EOF
#endif
	    || fclose(f) == EOF) {
		quit(errno, "%s: Write error.", (const char *) zname);
	}

	locked = 1;
	return;
}

file_lock::~file_lock() {
	if (locked) {
		locked = 0;
		unlink(name);
	}
}

#endif /* defined(CONFIG_PID_LOCKING) */




/* Local variables: */
/* mode: c++ */
/* End: */
