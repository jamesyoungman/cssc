/*
 * sysdep.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Includes system dependent header files.
 *
 * @(#) MySC sysdep.h 1.2 93/11/10 04:48:37
 *
 */

#ifndef __SYSDEP_H__
#define __SYSDEP_H__

#ifdef CONFIG_INCLUDE_FCNTL_H 
#include <fcntl.h>
#endif

#ifdef CONFIG_INCLUDE_SYS_FILE_H
#include <sys/file.h>
#endif

#ifdef CONFIG_INCLUDE_UNISTD_H
#include <unistd.h>
#endif

#ifdef CONFIG_INCLUDE_PROTOTYPES_H
#include <prototypes.h>
#endif

#ifdef CONFIG_INCLUDE_IO_H
#include <io.h>
#endif

#ifdef CONFIG_INCLUDE_SYS_WAIT_H
#include <sys/wait.h>
#ifdef CONFIG_WAIT_IS_A_USELESS_MACRO
#undef wait
#endif
#endif

#ifdef CONFIG_INCLUDE_PROCESS_H
#include <process.h>
#endif

#ifdef CONFIG_MSDOS_FILES
#if defined(__BORLANDC__) && defined(__STDC__)
#define far __far
#endif
#include <dos.h> 
#else
#include <sys/stat.h>
#endif

#ifdef CONFIG_UIDS
#include <pwd.h>
#endif

#ifdef CONFIG_SHARE_LOCKING
#include <share.h>
#endif

#ifdef CONFIG_DECLARE_FDOPEN
extern "C" FILE * CDECL fdopen(int, const char *);
#endif

#endif /* __SYSDEP_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
