/* -*- C++ -*-
 * The contents of this header file is only used if
 * strerror() is not provided by the implementation.
 * It either includes the appropriate header files
 * or declares sys_nerr and sys_errlist itself.
 */


#ifndef HAVE_STRERROR
/* Provide declaration of sys_nerr. 
 */
#ifdef HAVE_SYS_NERR
#if ERRNO_H_DECLARES_SYS_NERR 
#include <errno.h>
#elif STDIO_H_DECLARES_SYS_NERR
#include <stdio.h>
#elif STDLIB_H_DECLARES_SYS_NERR
#include <stdlib.h>
#else
extern int sys_nerr;
#endif
#endif


/* Provide declaration of sys_errlist. 
 */
#ifdef HAVE_SYS_ERRLIST
#if ERRNO_H_DECLARES_SYS_ERRLIST
#include <errno.h>
#elif STDIO_H_DECLARES_SYS_ERRLIST
#include <stdio.h>
#elif STDLIB_H_DECLARES_SYS_ERRLIST
#include <stdlib.h>
#else
extern char *sys_errlist[];
#endif
#endif


#endif
