/* Top of my file */
@TOP@


/* This file is like the acconfig.h file provided by Autoconf,
 * but explains those #defines that are local to this package.
 */

/* Location of the "diff" command. */
#undef CONFIG_DIFF_COMMAND


@BOTTOM@


/* This section contains those comments and definitions that are too
 * complex for autoheader.m4f to understand; autoheader works by
 * redefining the autoconf primitives in such a way that it gains a
 * record of what might or might not get defined, just by m4 expansion
 * of congigure.in.  It doesn't seem clever enough to cope with the
 * macros AC_CHECK_GLOBAL and AC_CHECK_DECL_IN_HEADER, because both of
 * those use AC_CACHE_VAL, which forces us to use a two-stage process
 * of getting the cached value or figuring it out, and then calling
 * AC_DEFINE etc. according to the result.   It might be possible to
 * produce a local m4 file that provided appropriate definitions of
 * the macros in configure.in which helped acheader out in this, but I
 * can't see how to do it.   Hence we work around this feature of
 * autoheader in this way.
 */

/* define if you have the ISO C function fsetpos(). */
#undef HAVE_FSETPOS

/* define if you have the global variable timezone. */
#undef HAVE_TIMEZONE

/* define if you have the global variable sys_nerr. */
#undef HAVE_SYS_NERR

/* define if you have the global variable sys_errlist. */
#undef HAVE_SYS_ERRLIST

/* define if <errno.h> declares sys_nerr. */
#undef ERRNO_H_DECLARES_SYS_NERR

/* define if <stdio.h> declares sys_nerr. */
#undef STDIO_H_DECLARES_SYS_NERR

/* define if <errno.h> declares sys_errlist. */
#undef ERRNO_H_DECLARES_SYS_ERRLIST

/* define if <stdio.h> declares sys_errlist. */
#undef STDIO_H_DECLARES_SYS_ERRLIST

/* define if <stdlib.h> declares sys_errlist. */
#undef STDLIB_H_DECLARES_SYS_ERRLIST


/* Bottom of my file (stop). */
