/*
 * file.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Declares system dependent routines for accessing files.
 *
 * @(#) MySC file.h 1.1 93/11/09 17:17:46
 *
 */

#ifndef __FILE_H__
#define __FILE_H__

#include "filelock.h"

enum create_mode {
	CREATE_EXCLUSIVE     = 001,
	CREATE_READ_ONLY     = 002,
	CREATE_AS_REAL_USER  = 004,
	CREATE_FOR_UPDATE    = 010,
	CREATE_FOR_GET       = 020,
#ifdef CONFIG_SHARE_LOCKING
	CREATE_WRITE_LOCK    = 040,
#endif
};

void stdout_to_null();
FILE *stdout_to_stderr();
int stdin_is_a_tty();
FILE *open_null();
int is_readable(char const *name);
int file_exists(char const *name);
char const *get_user_name();
int user_is_group_member(int gid);
FILE *fcreate(string name, int mode);

#ifdef CONFIG_SYNC_BEFORE_REOPEN
int ffsync(FILE *f);
#endif

#ifdef CONFIG_UIDS
void give_up_privileges();
void restore_privileges();
#else
inline void give_up_privileges() {}
inline void restore_privileges() {}
#endif

#ifdef CONFIG_NO_REMOVE
#undef remove
#define remove LIDENT(remove)
int remove(char const *name);
#endif

#ifdef CONFIG_NO_RENAME
#undef rename
#define rename LIDENT(rename)
int rename(char const *from, char const *to);
#endif

#ifdef CONFIG_USE_ARCHIVE_BIT
void clear_archive_bit(const char *name);
#endif

#endif /* __FILE_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
