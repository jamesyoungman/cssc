/*
 * Copyright (C) 1998, 1999, 2001, 2019, 2024 Free Software Foundation,
 * Inc.  All rights reserved.
 *
 * Copyright (c) 1980, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/* See also the file COPYING.bsd. */

#define DEBUG (1)

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1980, 1993\n"
"The Regents of the University of California.  All rights reserved.\n"
"@(#) Copyright (c) 1998\n"
"Free Software Foundation, Inc.  All rights reserved.\n";
#endif /* not lint */
static const char filever[] = "$Id: sccs.c,v 1.44 2007/12/19 00:21:14 jay Exp $";

#include <config.h>

#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/param.h>          /* TODO: this does what? */
#include <sys/stat.h>
#include <signal.h>             /* TODO: consider using sigaction(). */
#include <errno.h>              /* TODO: same as in parent directory. */
#include <pwd.h>                /* getpwuid() */
#ifdef HAVE_GRP_H
#include <grp.h>		/* setgroups() */
#endif
#include <gettext.h>

/* #include "pathnames.h" */
/* The next few lines inserted from @(#)pathnames.h     8.1 (Berkeley) 6/6/93
 */
/* #include <paths.h> */

#include "dirent-safer.h"
#include "progname.h"

#ifndef _PATH_BSHELL
#define _PATH_BSHELL "/bin/sh"
#endif

/* #define PASTE(a,b) a##b */

#define _PATH_SCCSADMIN ("admin")
#define _PATH_SCCSBDIFF ("bdiff")
#define _PATH_SCCSCOMB  ("comb")
#define _PATH_SCCSDELTA ("delta")
#define _PATH_SCCSDIFF  ("sccsdiff")
#define _PATH_SCCSGET   ("get")
#define _PATH_SCCSUNGET ("unget")
#define _PATH_SCCSHELP  ("help")
#define _PATH_SCCSPRS   ("prs")
#define _PATH_SCCSPRT   ("prt")
#define _PATH_SCCSRMDEL ("rmdel")
#define _PATH_SCCSCDC   ("cdc")
#define _PATH_SCCSVAL   ("val")
#define _PATH_SCCSWHAT  ("what")
#undef _PATH_TMP
#define _PATH_TMP       "/tmp/sccsXXXXX"

/* End of insertion from pathnames.h */


#ifndef PREFIX
#define PREFIX "/usr/sccs/"
#endif


#ifdef STAT_MACROS_BROKEN
#undef S_ISDIR
#endif


/*
 * Exit values; the <sysexits.h> file normally defines
 * EX_*, but on Solaris machines this seems to result
 * in redefinition of the values (at least in one reported
 * case, with GCC as the compiler).  To avoid this we define them
 * here, but with a name change to avoid clashes.
 */
#define CSSC_EX_OK              0  /* successful termination */
#define CSSC_EX_USAGE           64 /* command line usage error */
#define CSSC_EX_DATAERR         65 /* data format error */
#define CSSC_EX_NOINPUT         66 /* cannot open input */
#define CSSC_EX_NOUSER          67 /* addressee unknown */
#define CSSC_EX_NOHOST          68 /* host name unknown */
#define CSSC_EX_UNAVAILABLE     69 /* service unavailable */
#define CSSC_EX_SOFTWARE        70 /* internal software error */
#define CSSC_EX_OSERR           71 /* system error (e.g., can't fork) */
#define CSSC_EX_OSFILE          72 /* critical OS file missing */
#define CSSC_EX_CANTCREAT       73 /* can't create (user) output file */
#define CSSC_EX_IOERR           74 /* input/output error */
#define CSSC_EX_TEMPFAIL        75 /* temp failure; user is invited to retry */
#define CSSC_EX_PROTOCOL        76 /* remote error in protocol */
#define CSSC_EX_NOPERM          77 /* permission denied */
#define CSSC_EX_CONFIG          78 /* configuration error */



/*
   **  SCCS.C -- human-oriented front end to the SCCS system.
   **
   **   Without trying to add any functionality to speak of, this
   **   program tries to make SCCS a little more accessible to human
   **   types.  The main thing it does is automatically put the
   **   string "SCCS/s." on the front of names.  Also, it has a
   **   couple of things that are designed to shorten frequent
   **   combinations, e.g., "delget" which expands to a "delta"
   **   and a "get".
   **
   **   This program can also function as a setuid front end.
   **   To do this, you should copy the source, renaming it to
   **   whatever you want, e.g., "syssccs".  Change any defaults
   **   in the program (e.g., syssccs might default -d to
   **   "/usr/src/sys").  Then recompile and put the result
   **   as setuid to whomever you want.  In this mode, sccs
   **   knows to not run setuid for certain programs in order
   **   to preserve security, and so forth.
   **
   **   Note that this is not the original version of the code and so
   **   any assumptions you might make about the code security of the
   **   original code do not apply to this version.  Think twice (and
   **   define SCCSDIR) before installing this program setuid.
   **
   **   Usage:
   **           sccs [flags] command [args]
   **
   **   Flags:
   **           -d<dir>         <dir> represents a directory to search
   **                           out of.  It should be a full pathname
   **                           for general usage.  E.g., if <dir> is
   **                           "/usr/src/sys", then a reference to the
   **                           file "dev/bio.c" becomes a reference to
   **                           "/usr/src/sys/dev/bio.c".
   **           -p<path>        prepends <path> to the final component
   **                           of the pathname.  By default, this is
   **                           "SCCS".  For example, in the -d example
   **                           above, the path then gets modified to
   **                           "/usr/src/sys/dev/SCCS/s.bio.c".  In
   **                           more common usage (without the -d flag),
   **                           "prog.c" would get modified to
   **                           "SCCS/s.prog.c".  In both cases, the
   **                           "s." gets automatically prepended.
   **           -r              run as the real user.
   **
   **   Flags peculiar to the GNU version of this program:
   **
   **           --cssc           Print "yes", and exit successfully.
   **           --version        Show version information.
   **           --prefix=foo     Prepend ``foo'' to the names of the
   **                            sccs programs before running them.
   **                            If --prefix is not used, there is a
   **                            default, usually "/usr/sccs".  Use the
   **                            --version flag to discover the default
   **                            prefix.
   **           -V               Synonymous with --version.
   **
   **   Commands:
   **           admin,
   **           get,
   **           unget,
   **           delta,
   **           rmdel,
   **           cdc,
   **           etc.            Straight out of SCCS; only difference
   **                           is that pathnames get modified as
   **                           described above.
   **           enter           Front end doing "sccs admin -i<name> <name>"
   **           create          Macro for "enter" followed by "get".
   **           edit            Macro for "get -e".
   **           unedit          Removes a file being edited, knowing
   **                           about p-files, etc.
   **           delget          Macro for "delta" followed by "get".
   **           deledit         Macro for "delta" followed by "get -e".
   **           branch          Macro for "get -b -e", followed by "delta
   **                           -s -n", followd by "get -e -t -g".
   **           diffs           "diff" the specified version of files
   **                           and the checked-out version.
   **           print           Macro for "prs -e" followed by "get -p -m".
   **           tell            List what files are being edited.
   **           info            Print information about files being edited.
   **           clean           Remove all files that can be
   **                           regenerated from SCCS files.
   **           check           Like info, but return exit status, for
   **                           use in makefiles.
   **           fix             Remove a top delta & reedit, but save
   **                           the previous changes in that delta.
   **
   **   Compilation Flags:
   **           SCCSDIR -- if defined, forces the -d flag to take on
   **                   this value.  This is so that the setuid
   **                   aspects of this program cannot be abused.
   **                   This flag also disables the -p flag.
   **                   Setuid execution is only supported if this
   **                   flag is set.
   **           SCCSPATH -- the default for the -p flag.
   **           MYNAME -- the title this program should print when it
   **                   gives error messages.
   **
   **           UIDUSER -- removed since we cannot trust $USER
   **                      or getlogin in setuid programs.
   **
   **           PREFIX  -- Sets the default prefix which allows us to
   **                      find the SCCS subcommands.  Unless you overrride
   **                      this on the compiler command line or by editing
   **                      the source, this defaults to "/usr/sccs".  Using
   **                      the --version flag will tell you what the setting
   **                      is.   This macro is available only in the GNU
   **                      version of this program.
   **
   **
   **
   **   Compilation Instructions:
   **           cc -O -n -s sccs.c
   **           The flags listed above can be -D defined to simplify
   **                   recompilation for variant versions.
   **
   **   Author:
   **           Eric Allman, UCB/INGRES
   **           Copyright 1980 Regents of the University of California
 */


/*******************  Configuration Information  ********************/

#ifndef SCCSPATH
#define SCCSPATH        "SCCS"  /* pathname in which to find s-files */
#endif

#ifndef MYNAME
#define MYNAME          "sccs"  /* name used for printing errors */
#endif

/****************  End of Configuration Information  ****************/

typedef char bool;
#define TRUE    1
#define FALSE   0

#define bitset(bit, word)       ((bool) ((bit) & (word)))

struct sccsprog
  {
    const char *sccsname;       /* name of SCCS routine */
    short sccsoper;             /* opcode, see below */
    short sccsflags;            /* flags, see below */
    const char *sccspath;       /* pathname of binary implementing */
    int   clean_mode;           /* mode for do_clean(). */
  };

/* values for sccsoper */
#define PROG            0       /* call a program */
#define CMACRO          1       /* command substitution macro */
#define FIX             2       /* fix a delta */
#define CLEAN           3       /* clean out recreatable files */
#define UNEDIT          4       /* unedit a file */
#if 0
#define SHELL           5       /* call a shell file (like PROG) */
#endif
#define DIFFS           6       /* diff between sccs & file out */
#define DODIFF          7       /* internal call to diff program */
#define ENTER           8       /* enter new files */

/* bits for sccsflags */
#define NO_SDOT 0001            /* no s. on front of args */
#define REALUSER        0002    /* protected (e.g., admin) */
#define WARN_MISSING    0004    /* not implemented in GNU CSSC */
#define NO_HELP_HERE    0010    /* issue special message re "help" */

/* modes for the "clean", "info", "check" ops */
#define CLEANC          0       /* clean command */
#define INFOC           1       /* info command */
#define CHECKC          2       /* check command */
#define TELLC           3       /* give list of files being edited */

/*
   **  Description of commands known to this program.
   **   First argument puts the command into a class.  Second arg is
   **   info regarding treatment of this command.  Third arg is a
   **   list of flags this command accepts from macros, etc.  Fourth
   **   arg is the pathname of the implementing program, or the
   **   macro definition, or the arg to a sub-algorithm.
 */

const struct sccsprog SccsProg[] =
{
  {"admin", PROG, REALUSER, _PATH_SCCSADMIN, 0 },
  {"cdc", PROG, 0, _PATH_SCCSCDC, 0 },
  {"comb", PROG, WARN_MISSING, _PATH_SCCSCOMB, 0 },
  {"delta", PROG, 0, _PATH_SCCSDELTA, 0 },
  {"get", PROG, 0, _PATH_SCCSGET, 0 },
  {"unget", PROG, 0, _PATH_SCCSUNGET, 0 },
  {"help", PROG, NO_HELP_HERE | NO_SDOT, _PATH_SCCSHELP, 0 },
  {"prs", PROG, 0, _PATH_SCCSPRS, 0 },
  {"prt", PROG, 0, _PATH_SCCSPRT, 0 },
  {"rmdel", PROG, REALUSER, _PATH_SCCSRMDEL, 0 },
  {"val", PROG, 0, _PATH_SCCSVAL, 0 },
  {"what", PROG, NO_SDOT, _PATH_SCCSWHAT, 0 },
  {"sccsdiff", PROG, REALUSER, _PATH_SCCSDIFF, 0 },
  {"edit", CMACRO, NO_SDOT, "get -e", 0 },
  {"delget", CMACRO, NO_SDOT, "delta:mysrp/get:ixbeskcl -t", 0 },
  {"deledit", CMACRO, NO_SDOT, "delta:mysrp -n/get:ixbskcl -e -t -g", 0 },
  {"fix", FIX, NO_SDOT, NULL, 0 },
  {"clean", CLEAN, REALUSER | NO_SDOT, (char *) NULL, CLEANC },
  {"info", CLEAN, REALUSER | NO_SDOT, (char *) NULL, INFOC },
  {"check", CLEAN, REALUSER | NO_SDOT, (char *) NULL, CHECKC },
  {"tell", CLEAN, REALUSER | NO_SDOT, (char *) NULL, TELLC },
  {"unedit", UNEDIT, NO_SDOT, NULL, 0 },
  {"diffs", DIFFS, NO_SDOT | REALUSER, NULL, 0 },
  {"-diff", DODIFF, NO_SDOT | REALUSER, _PATH_SCCSBDIFF, 0 },
  {"print", CMACRO, NO_SDOT, "prs -e/get -p -m -s", 0 },
  {"branch", CMACRO, NO_SDOT, "get:ixrc -e -b/delta: -s -n -ybranch-place-holder/get:pl -e -t -g", 0 },
  {"enter", ENTER, NO_SDOT, NULL, 0 },
  {"create", CMACRO, NO_SDOT, "enter/get:ixeskcl -t", 0 },
  {NULL, -1, 0, NULL, 0 },
};

/* one line from a p-file */
struct pfile
  {
    char *p_osid;               /* old SID */
    char *p_nsid;               /* new SID */
    char *p_user;               /* user who did edit */
    char *p_date;               /* date of get */
    char *p_time;               /* time of get */
    char *p_aux;                /* extra info at end */
  };

const char *SccsPath = SCCSPATH;        /* pathname of SCCS files */
#ifdef SCCSDIR
const char *SccsDir = SCCSDIR;  /* directory to begin search from */
#else
const char *SccsDir = "";
#endif
char *subprogram_exec_prefix = NULL; /* see try_to_exec(). */

int OutFile = -1;               /* override output file for commands */
bool RealUser;                  /* if set, running as real user */
#ifdef DEBUG
bool Debug = 0;                 /* turn on tracing */
#endif
static bool TrustEnvironment = 0;


void syserr (const char *fmt,...);
void usrerr (const char *fmt,...);
int command (char *argv[], bool forkflag, const char *arg0);
int callprog (const char *progpath, short flags,
              char *const argv[], bool forkflag);
int clean (int mode, char *const argv[]);
int dodiff (char * getv[], const char *gfile);
int isbranch (const char *sid);
void putpfent (register const struct pfile *pf, register FILE * f);
bool safepath (register const char *p);
bool isdir (const char *name);
const struct sccsprog *lookup (const char *name);
bool unedit (const char *fn);
char *makefile (const char *name);
const char *tail (register const char *fn);
char *tail_nc (register char *fn);
const struct pfile *getpfent (FILE * pfp);
const char *username (void);
char *nextfield (register char *p);
char *my_rindex(const char *s, int ch);
char *my_index(const char *s, int ch);


static char *gstrcat (char *to, const char *from, size_t length);
static char *gstrncat (char *to, const char *from, size_t n, size_t length);
static char *gstrcpy (char *to, const char *from, size_t length);
static void gstrbotch (const char *str1, const char *str2);
static void  gstrbotchn (int avail, const char *, int, const char *, int);
static int absolute_pathname (const char *);

static char *str_dup (const char *);
static void childwait(int pid, int *status_ptr, int ignoreintr);


/* #define      FBUFSIZ BUFSIZ */

#define FBUFSIZ (1024u)

static void
show_version(void)
{
  fprintf(stderr, "%s from GNU CSSC %s\n%s\n", program_name, (VERSION), filever);
  fprintf(stderr, "SccsPath = '%s'\nSccsDir = '%s'\n", SccsPath, SccsDir);
  fprintf(stderr, "Default prefix for SCCS subcommands is '%s'\n", (PREFIX));
}


static void oom(void)
{
  perror("malloc failed");
  exit(CSSC_EX_TEMPFAIL);
}

/* set_prefix()
 *
 * The user has specified the --prefix option which indicates
 * where to look for the subcommands.   This is primarily
 * used by the test suite so that it can be run before
 * the programs have been installed.
 *
 * However, if the user is allowed to provide an arbitrary prefix,
 * they could use this facility to execute arbitrary programs.
 * If this program is installed setuid, that is a security hole.
 *
 * I'm not saying that CSSC isn't one big security hole itself,
 * but we should certainly forbid this.
 *
 * See also try_to_exec().
 */
static void
set_prefix(const char *pfx)
{
  if (TrustEnvironment)
    {
      char *p = malloc(1+strlen(pfx));
#ifdef DEBUG
      if (Debug)
        printf ("set_prefix: setting execution prefix to '%s'\n", pfx);
#endif
      if (p)
        {
	  free (subprogram_exec_prefix);
          strcpy(p, pfx);
          subprogram_exec_prefix = p;
        }
      else
        {
          oom();
        }
    }
  else
    {
      fprintf(stderr,
              "%s",
              "Option --prefix is incompatible with setuid "
              "execution.  Sorry.\n");
      exit (CSSC_EX_USAGE);
    }
}

static void
setuid_warn(void)
{
  const char *str =
    "If you want to install this program set-user-id or set-group-id, you\n"
    "must compile it with the SCCSDIR macro defined, in order to\n"
    "prevent abuse.  Even so, abuse is probably not impossible.  This\n"
    "is not a recommended mode of operation for this program.\n";
  fprintf(stderr, "%s", str);
}


static void
drop_privs(void)
{
  /* Call setgroups() before setuid(), see POS36-C */
#ifdef HAVE_SETGROUPS
  /* EPERM can be ignored here - if we can't call setgroups(), we're safe */
  if (0 != setgroups(0, NULL) && errno != EPERM)
    {
      perror("setgroups");
      exit(CSSC_EX_NOPERM);
    }
#endif
  /* Call setgid() before setuid(), see POS36-C */
  if (0 != setgid(getgid()))
    {
      perror("setgid");
      exit(CSSC_EX_NOPERM);
    }
  if (0 != setuid (getuid ()))
    {
      perror("setuid");
      exit(CSSC_EX_NOPERM);
    }
  RealUser++;
}

static void
check_data_integrity()
{
}

/* cleanup_environment
 *
 * This function unsets the CSSC configuration variables, so that the values
 * specified to "configure" at compilation time are enforced.  This prevents
 * the invoking user overriding the policy decision made by the person who
 * installed the set-user-id or set-group-id copy of the "sccs" driver.
 *
 * This function is not called when "sccs" is running unprivileged.
 */
static void cleanup_environment(void)
{
  const char * binary_support = "CSSC_BINARY_SUPPORT";
  const char * max_line_len   = "CSSC_MAX_LINE_LENGTH";
#ifdef HAVE_UNSETENV
  unsetenv(binary_support);
  unsetenv(max_line_len);
#else

  /* XXX: not ideal.  We'd like just to turn them off, but
   * if we have no unsetenv(), we simply have to fail.
   */
  pfail = getenv(binary_support);
  if (NULL == pfail)
    pfail = getenv(max_line_len);

  if (pfail)
    {
      fprintf(stderr,
              "You should not set the %s environment variable when "
              "the sccs driver is running set-user-id or set-group-id.\n",
              pfail);
      exit(CSSC_EX_NOPERM);
    }
#endif
}

static void
usage(void)
{
  fprintf (stderr, "Usage: %s [flags] command [flags]\n", program_name);
}

char * my_rindex(const char *p, int ch)
{
  return strrchr(p, ch);
}


char * my_index(const char *p, int ch)
{
  return strchr(p, ch);
}



#define PFILELG 120

int
main (int argc, char **argv)
{
  register char *p;
  register int i;
  int hadver = 0;

  (void) &copyright;            /* prevent warning about unused variable. */

  set_program_name (argv[0]);

  if (!absolute_pathname(PREFIX))
    {
      fprintf(stderr,
              "Compiled-in program name prefix %s is not absolute.\n"
              "Please recompile this program using an absolute path.\n",
              PREFIX);
      exit(CSSC_EX_CONFIG);
    }

  if ( (getuid() != geteuid()) || (getgid() != getegid()))
    {
      TrustEnvironment = 0;     /* running setuid, ignore $PATH etc. */
#ifndef SCCSDIR
      setuid_warn();
      exit(CSSC_EX_NOPERM);
#endif
      cleanup_environment();
    }
  else
    {
      TrustEnvironment = 1;
    }

  if (TrustEnvironment)
    {
#ifdef HAVE_SETLOCALE
#ifndef __ultrix
      /* Mark Reynolds <mark@aoainc.com>: If $LANG is not
       * set, setlocale() fails on VAX Ultrix 4.2.
       */
      if (NULL == setlocale(LC_ALL, ""))
        {
          /* If we can't set the locale as the user wishes,
           * emit an error message and continue.   The error
           * message will of course be in the "C" locale.
           */
          perror("Error setting locale");
        }
#endif
#endif
    }

  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  check_data_integrity();

#ifndef SCCSDIR

  /* Setuid execution is only allowed if SCCSDIR is defined,
   * so we can "trust" the PROJECTDIR environment variable.
   */

  /* pull "SccsDir" out of the environment (possibly) */
  p = getenv("PROJECTDIR");
  if (p != NULL && p[0] != '\0')
    {
      register struct passwd *pw;
      extern struct passwd *getpwnam();
      char buf[FBUFSIZ];

      if (p[0] == '/')
        SccsDir = p;
      else
        {
          pw = getpwnam(p);
          if (pw == NULL)
            {
              usrerr("user %s does not exist", p);
              exit(CSSC_EX_USAGE);
            }
          gstrcpy(buf, pw->pw_dir, sizeof(buf));
          gstrcat(buf, "/src", sizeof(buf));
          if (access(buf, 0) < 0)
            {
              gstrcpy(buf, pw->pw_dir, sizeof(buf));
              gstrcat(buf, "/source", sizeof(buf));
              if (access(buf, 0) < 0)
                {
                  usrerr("project %s has no source!", p);
                  exit(CSSC_EX_USAGE);
                }
            }
          SccsDir = buf;
        }
    }
#endif


  /*
     **  Detect and decode flags intended for this program.
   */

  if (argc < 2)
    {
      usage();
      exit (CSSC_EX_USAGE);
    }
  argv[argc] = NULL;

  if (lookup (argv[0]) == NULL)
    {
      while ((p = *++argv) != NULL)
        {
          if (*p != '-')
            break;
          switch (*++p)
            {
            case '-': /* long option. */
              ++p;
              if (0 == *p)      /* just "--" */
                {
                  fprintf(stderr, "%s",
                          "End-of-arguments option \"--\" not "
                          "supported, sorry.\n");
                  exit (CSSC_EX_USAGE);
                }
              else if (0 == strncmp(p, "prefix=", 7))
                {
                  set_prefix(p+7);
                }
              else if (0 == strcmp(p, "cssc"))
                {
                  printf("%s\n", "yes");
                  exit(CSSC_EX_OK);
                }
              else if (0 == strcmp(p, "version"))
                {
                  if (!hadver)
                    show_version();
                  hadver = 1;
                  if (2 == argc) /* If the only arg, return success. */
                    return 0;
                }
              else
                {
                  usrerr ("unknown option --%s", p);
                  usage();
                  exit (CSSC_EX_USAGE);
                }
              break;

            case 'V':
              if (!hadver)
                show_version();
              hadver = 1;
              if (2 == argc)    /* If -V was the only arg, return success. */
                return 0;
              break;            /* Otherwise, process the remaining options. */

            case 'r':           /* run as real user */
              drop_privs();
              break;

#ifndef SCCSDIR
            case 'p':           /* path of sccs files */
              SccsPath = ++p;
              if (SccsPath[0] == '\0' && argv[1] != NULL)
                SccsPath = *++argv;
              break;

            case 'd':           /* directory to search from */
              SccsDir = ++p;
              if (SccsDir[0] == '\0' && argv[1] != NULL)
                SccsDir = *++argv;
              break;
#else
            case 'p':
            case 'd':
              fprintf(stderr, "The %c option has been disabled.\n", *p);
              exit(CSSC_EX_USAGE);
              break;
#endif

            case 'T':           /* trace */
#ifdef DEBUG
              Debug++;
#else
              fprintf(stderr, "%s",
                      "The -T option has been disabled.  Sorry.\n");
              exit(CSSC_EX_USAGE);
#endif
              break;

            default:
              usrerr ("unknown option -%s", p);
              usage();
              exit (CSSC_EX_USAGE);
            }
        }
      if (SccsPath[0] == '\0')
        SccsPath = ".";
    }

  if (NULL == argv[0])
    {
      /* No remaining args!
       */
      if (!hadver)
        {
          /* Not sure what went on, but it wasn't a useful command line. */
          usage();
	  return 1;
        }
      else
        {
          /* Just "sccs -V" is valid. */
          return 0;
        }
    }
  else
    {
      i = command (argv, FALSE, "");
      return i;
    }
}

char ** do_enter(char *argv[], char **np, char **ap,
                 int *rval)
{
  char buf2[FBUFSIZ];
  char *argv_tmp;

  /* skip over flag arguments */
  for (np = &ap[1]; *np != NULL && **np == '-'; np++)
    continue;
  argv = np;

  /* do an admin for each file */
  argv_tmp = argv[1];
  while (np[0] != NULL)
    {
      printf ("\n%s:\n", *np);
      strcpy (buf2, "-i");
      gstrcat (buf2, np[0], sizeof (buf2));
      ap[0] = buf2;     /* sccs enter foo --> admin -ifoo */
      argv[0] = tail_nc (np[0]);
      argv[1] = NULL;
      *rval = command (ap, TRUE, "admin");

      argv[1] = argv_tmp;
      if (*rval == 0)
        {
          strcpy (buf2, ",");
          gstrcat (buf2, tail (np[0]), sizeof (buf2));
          if (link (np[0], buf2) >= 0)
            unlink (np[0]);
        }
      np++;
    }
  return np;
}

static int
absolute_pathname(const char *p)
{
  return '/' == p[0];
}


static void
try_to_exec(const char *prog, char * const argv[])
{
  char *newprog;
  const char *prefix;
  size_t len;

#ifdef DEBUG
  if (Debug)
    printf ("try_to_exec: %s\n", prog);
#endif

  /* subprogram_exec_prefix is always NULL if we are runnign setuid. */
  if (subprogram_exec_prefix)
    {
      prefix = subprogram_exec_prefix;
#ifdef DEBUG
      if (Debug)
        printf ("try_to_exec: Using user prefix '%s'\n", prefix);
#endif
    }
  else
    {
      prefix = (PREFIX);
#ifdef DEBUG
      if (Debug)
        printf ("try_to_exec: Using default prefix '%s'\n", prefix);
#endif


      /* If no custom prefix was specified on the command line,
       * we start by using execvp, because:-
       *
       * 1) The user may wish to use SCCS programs other than those
       *    in a fixed location.
       * 2) execv("prt") will exec a program "prt" in the current
       *    dorectory (execvp will not).
       */

       /* Honour $PATH unless running setuid.
        * Must NOT use execvp() if running setuid.
        */
      if (TrustEnvironment || RealUser)
        execvp(prog, argv);             /* execvp() uses $PATH */
    }

  /* absolute_pathname() does not call a library function, so no need
   * to save/restore errno.
   */
  if (absolute_pathname(prog))
    {
      execv(prog, argv);
      perror(prog);
    }
  else
    {
      /* if the above exec() returns or was not allowed, try our prefix.
       */
      /* SourceForge BUG #448215: patch from Jeff Sheinberg;
       * change seconf strlen call from strlen(prefix) to strlen(prog).
       */
      len = strlen(prefix) + strlen(prog);
      newprog = malloc(len + 1);
      if (NULL == newprog)
        {
          oom();
          /*NOTREACHED*/
          exit(CSSC_EX_TEMPFAIL);
        }
      sprintf(newprog, "%s%s", prefix, prog);
      prog = newprog;


#ifdef DEBUG
      if (Debug)
        printf ("try_to_exec: %s\n", prog);
#endif

      execv(prog, argv);
      perror(prog);
    }

  if (Debug)
    {
      fprintf(stderr, "try_to_exec: exec failed.\n");
    }
}

/*
   **  COMMAND -- look up and perform a command
   **
   **   This routine is the guts of this program.  Given an
   **   argument vector, it looks up the "command" (argv[0])
   **   in the configuration table and does the necessary stuff.
   **
   **   Parameters:
   **           argv -- an argument vector to process.
   **           forkflag -- if set, fork before executing the command.
   **           editflag -- if set, only include flags listed in the
   **                   sccsklets field of the command descriptor.
   **           arg0 -- a space-seperated list of arguments to insert
   **                   before argv.
   **
   **   Returns:
   **           zero -- command executed ok.
   **           else -- error status.
   **
   **   Side Effects:
   **           none.
 */
/* Warning in command () */
int
command (char *argv[], bool forkflag, const char *arg0)
{
  const struct sccsprog *cmd;
  char buf[FBUFSIZ]; /* BUG: access to this is not bounds-checked. */
  char *nav[1000];
  char **np;
  char **ap;
  int rval = 0;                 /* value to be returned. */

#ifdef DEBUG
  if (Debug)
    {
      int i;
      printf ("command:\n\t\"%s\"\n", arg0);
      for (i=0; argv[i] != NULL; ++i)
        printf ("\t\"%s\"\n", argv[i]);
    }
#endif

  /*
     **  Copy arguments.
     ** Copy from arg0 & if necessary at most one arg
     ** from argv[0].
   */

  np = ap = &nav[1];
  if (1)                        /* introduce scope for editchs. */
    {
      char *editchs;

      editchs = NULL;
      if (1)                    /* introduce scope */
        {
          char *q;
          const char *p;

          for (p = arg0, q = buf; *p != '\0' && *p != '/';)
            {
              *np++ = q;
              while (*p == ' ')         /* wind p to next word. */
                p++;
              while (*p != ' ' && *p != '\0' && *p != '/' && *p != ':')
                *q++ = *p++;
              *q++ = '\0';
              if (*p == ':')
                {
                  editchs = q;
                  while (*++p != '\0' && *p != '/' && *p != ' ')
                    *q++ = *p;
                  *q++ = '\0';
                }
            }
        }

      *np = NULL;
      if (*ap == NULL)
        *np++ = *argv++;

      /*
      **  Look up command.
      ** At this point, *ap is the command name.
      */

      cmd = lookup (*ap);
      if (cmd == NULL)
        {
          usrerr ("Unknown command \"%s\"", *ap);
          usage();
          return (CSSC_EX_USAGE);
        }
      else
        {
          if (cmd->sccsflags & WARN_MISSING)
            {
              fprintf(stderr,
                      "Warning: the \"%s\" command is not yet implemented.\n",
                      *ap);
              /* continue anyway just in case we did implement it, or
               * (perhaps) there is a real SCCS around somewhere.
               */
            }
          if (cmd->sccsflags & NO_HELP_HERE)
            {
              fprintf(stderr,
                      "GNU CSSC does not provide the \"%s\" command;\n"
                      "please see the relevant entry in the GNU CSSC manual\n"
                      "and the \"Missing Features and other Problems\" chapter\n"
                      "in particular.\n\n",
                      *ap);
            }
        }


      /*
      **  Copy remaining arguments doing editing as appropriate.
      */

      for (; *argv != NULL; argv++)
        {
          char *p;

          p = *argv;
          if (*p == '-')
            {
              if (p[1] == '\0' || editchs == NULL || my_index (editchs, p[1]) != NULL)
                *np++ = p;
            }
          else
            {
              if (!bitset (NO_SDOT, cmd->sccsflags))
                p = makefile (p);       /* MEMORY LEAK (of returned value) */
              if (p != NULL)
                *np++ = p;
            }
        }
      *np = NULL;
    }

  /*
     **  Interpret operation associated with this command.
   */

  switch (cmd->sccsoper)
    {
#if 0
    case SHELL:         /* call a shell file */
      {
        ap[0]  = cmd->sccspath; /* Warning: discards const */
        ap[-1] = "sh";
        rval = callprog (_PATH_BSHELL, cmd->sccsflags, ap-1, forkflag);
      }
      break;
#endif

    case PROG:                  /* call an sccs prog */
      {
        rval = callprog (cmd->sccspath, cmd->sccsflags, ap, forkflag);
      }
      break;

    case CMACRO:                /* command macro */
      {
        const char *s;

        /* step through & execute each part of the macro */
        for (s = cmd->sccspath; *s != '\0'; s++)
          {
            const char *qq = s;
            while (*s != '\0' && *s != '/')
              s++;
            rval = command (&ap[1], *s != '\0', qq);
            if (rval != 0)
              break;
          }
      }
      break;

    case FIX:                   /* fix a delta */
      {
        if (ap[1] == 0 || strncmp (ap[1], "-r", 2) != 0)
          {
            usrerr ("-r flag needed for fix command");
            rval = CSSC_EX_USAGE;
            break;
          }

        /* get the version with all changes */
        rval = command (&ap[1], TRUE, "get -k");

        /* now remove that version from the s-file */
        if (rval == 0)
          rval = command (&ap[1], TRUE, "rmdel:r");

        /* and edit the old version (but don't clobber new vers) */
        if (rval == 0)
          rval = command (&ap[2], FALSE, "get -e -g");
      }
      break;

    case CLEAN:
      {
        rval = clean ( cmd->clean_mode, ap );
      }
      break;

    case UNEDIT:
      {
        for (argv = np = &ap[1]; *argv != NULL; argv++)
          {
            if (unedit (*argv))
              *np++ = *argv;
          }
	fflush (stdout);
        *np = NULL;

        /* Test difference: unedit() says " foo: removed" and this
         * output comes *after* the output from get.  This happens
         * when the output is a file.  it's a buffering issue, not
         * noticed by the casual user.
         *
         * Sigh.
         *
         * We can get the same output if we fork to run get; that way,
         * the parent's output remains in the stdout buffer until
         * after the child has exited.
         */

        /* get all the files that we unedited successfully */
        if (np > &ap[1])
          rval = command (&ap[1], TRUE, "get");
      }
      break;

    case DIFFS:         /* diff between s-file & edit file */
      {
        char *s;

        /* find the end of the flag arguments */
        for (np = &ap[1]; *np != NULL && **np == '-'; np++)
          continue;
        argv = np;

        /* for each file, do the diff */
        s = argv[1];
        while (*np != NULL)
          {
            int this_ret;
            /* messy, but we need a null terminated argv */
            *argv = *np++;
            argv[1] = NULL;
            this_ret = dodiff (ap, tail (*argv));
            if (rval == 0)
              rval = this_ret;
            argv[1] = s;
          }
      }
      break;

    case DODIFF:                /* internal diff call */
      {
        drop_privs();
        for (np = ap; *np != NULL; np++)
          {
            if ((*np)[0] == '-' && (*np)[1] == 'C')
              (*np)[1] = 'c';
          }

        /* insert "-" argument */
        np[1] = NULL;
        np[0] = np[-1];
        np[-1] = "-";

        /* execute the diff program of choice */
#ifndef V6
        if (TrustEnvironment)
          execvp ("diff", ap);
#endif
        try_to_exec (cmd->sccspath, argv);
        exit (CSSC_EX_OSERR);
      }
      /*NOTREACHED */
      break;

    case ENTER:         /* enter new sccs files */
      np = do_enter(argv, np, ap, &rval);
      break;

    default:
      {
        syserr ("Unexpected oper %d", cmd->sccsoper);
        exit (CSSC_EX_SOFTWARE);
      }
      /*NOTREACHED */
      break;
    }
#ifdef DEBUG
  if (Debug)
    printf ("command: rval=%d\n", rval);
#endif
  return rval;
}


/*
   **  LOOKUP -- look up an SCCS command name.
   **
   **   Parameters:
   **           name -- the name of the command to look up.
   **
   **   Returns:
   **           ptr to command descriptor for this command.
   **           NULL if no such entry.
   **
   **   Side Effects:
   **           none.
 */

const struct sccsprog *
lookup (const char *name)
{
  register const struct sccsprog *cmd;

  for (cmd = SccsProg; cmd->sccsname != NULL; cmd++)
    {
      if (strcmp (cmd->sccsname, name) == 0)
        return cmd;
    }
  return NULL;
}

/*
 * childwait()
 *
 * Wait for a child process, perhaps with SIGINT ignored.
 */
static void
childwait(int pid, int *status_ptr, int ignoreintr)
{
  struct sigaction sa, osa;
  int ret;

  /* temporarily ignore SIG_INT.
   */
  memset (&sa, 0, sizeof(sa));
  if (ignoreintr)
    {
      sa.sa_handler = SIG_IGN;
      sigemptyset(&sa.sa_mask);
      sa.sa_flags = 0;
      ret = sigaction(SIGINT, &sa, &osa);
    }
  /* May eventually need to kludge waitpid() as a loop
   * with wait() for older systems.
   */
  while ( -1 == waitpid(pid, status_ptr, 0) && EINTR == errno)
    errno = 0;

  /* Restore previous disposition of SIG_INT.
   */
  if (ignoreintr && 0 == ret)
    sigaction(SIGINT, &osa, NULL);
}


/*
 *  get_sig_name -- find the name for a signal.
 *
 */
static const char *
get_sig_name(unsigned int sig,
             char sigmsgbuf[11])
{
#ifdef SYS_SIGLIST_DECLARED
#ifdef NSIG
  if (sig < NSIG)
    return sys_siglist[sig];
#endif
#endif
  if (sig > 999) sig = 999;     /* prevent buffer overflow (!) */
  sprintf (sigmsgbuf, "Signal %u", sig);
  return sigmsgbuf;
}


/* do_fork()
 *
 * This function was created so that we could do things in preparation
 * for fork, specifically, ensure that output streams are flushed and
 * so on.   Unfortunately, if we flush our output before forking, we break
 * compatibility with some vendors' implementations of "sccs unedit", for
 * which the output of "get" comes before the "foo: removed" output.
 * The fact that that is stupid is beside the point. -- <jay@gnu.org>.
 */
static pid_t
do_fork(void)
{
  /* ? Sleep if we get EAGAIN? */
  /* ? Sleep if we get ENOMEM? */
  return fork();
}


/*
   **  CALLPROG -- call a program
   **
   **   Used to call the SCCS programs.
   **
   **   Parameters:
   **           progpath -- pathname of the program to call.
   **           flags -- status flags from the command descriptors.
   **           argv -- an argument vector to pass to the program.
   **           forkflag -- if true, fork before calling, else just
   **                   exec.
   **
   **   Returns:
   **           The exit status of the program.
   **           Nothing if forkflag == FALSE.
   **
   **   Side Effects:
   **           Can exit if forkflag == FALSE.
 */

int
callprog (const char *progpath,
          short flags,
          char *const argv[],
          bool forkflag)
{
  register int i;

#ifdef DEBUG
  if (Debug)
    {
      printf ("%s\n", "callprog:");
      for (i = 0; argv[i] != NULL; i++)
        printf ("\t\"%s\"\n", argv[i]);
    }
#endif

  if (*argv == NULL)
    return (-1);

  /*
     **  Fork if appropriate.
   */

  if (forkflag)
    {
#ifdef DEBUG
      if (Debug)
        printf ("%s", "Forking\n");
#endif
      i = do_fork ();
      if (i < 0)
        {
          syserr ("cannot fork");
          exit (CSSC_EX_OSERR);
        }
      else if (i > 0)           /* parent */
        {
          int st;

          childwait(i, &st, 0); /* don't block SIGINT. */

          if (WIFEXITED(st))    /* normal exit. */
            {
              st = WEXITSTATUS(st);
            }
          else                  /* child exited via signal */
            {
              int sigcode = WTERMSIG(st);
              if (sigcode != SIGINT && sigcode != SIGPIPE)
                {
                  char sigmsgbuf[11];
                  fprintf (stderr,
                           "%s: %s: %s%s\n",
                           program_name,
                           argv[0],
                           get_sig_name(sigcode, sigmsgbuf),
                           (WCOREDUMP(st) ? " (core dumped)" : "") );
                }
              st = CSSC_EX_SOFTWARE;
            }

          if (OutFile >= 0)
            {
              close (OutFile);
              OutFile = -1;
            }
          return (st);
        }
    }
  else if (OutFile >= 0)
    {                              /* TODO: make this impossible. */
      syserr ("callprog: setting stdout without forking");
      exit (CSSC_EX_SOFTWARE);
    }

  /*
   * in child (or didn't fork at all).
   */

  /* set protection as appropriate */
  if (bitset (REALUSER, flags))
    {
      drop_privs();
      RealUser = 1;
#ifdef DEBUG
      if (Debug)
        printf ("callprog: gave up privileges.\n");
#endif
    }

  /* change standard input & output if needed */
  if (OutFile >= 0)
    {
      close (1);
      dup (OutFile);
      close (OutFile);
    }

  /* call real SCCS program */
  try_to_exec (progpath, argv);
  exit (CSSC_EX_UNAVAILABLE);
  /*NOTREACHED */
}

/*
   **  STR_DUP -- make a copy of a string.
   **
   **   Parameters:
   **           S -- the string to be cpied.
   **
   **   Returns:
   **           A duplicate copy of that string.
   **           NULL on error.
   **
   **   Side Effects:
   **           none.
 */

static char *
str_dup (const char *s)
{
  char *p;
  size_t len = strlen (s) + 1u; /* include space for terminating '\0' */
  p = malloc (len);
  if (p)
    {
      memcpy (p, s, len);
    }
  else
    {
      perror ("Sccs: no mem");
      exit (CSSC_EX_TEMPFAIL);
    }
  return p;
}


/*
   **  MAKEFILE -- make filename of SCCS file
   **
   **   If the name passed is already the name of an SCCS file,
   **   just return it.  Otherwise, munge the name into the name
   **   of the actual SCCS file.
   **
   **   There are cases when it is not clear what you want to
   **   do.  For example, if SccsPath is an absolute pathname
   **   and the name given is also an absolute pathname, we go
   **   for SccsPath (& only use the last component of the name
   **   passed) -- this is important for security reasons (if
   **   sccs is being used as a setuid front end), but not
   **   particularly intuitive.
   **
   **   Parameters:
   **           name -- the file name to be munged.
   **
   **   Returns:
   **           The pathname of the sccs file.
   **           NULL on error.
   **
   **   Side Effects:
   **           none.
 */

char *
makefile (const char *name)
{
  register const char *p;
  char buf[3 * FBUFSIZ];
  register char *q;
  size_t left;

  p = my_rindex (name, '/');
  if (p == NULL)
    p = name;
  else
    p++;

  /*
     **  Check to see that the path is "safe", i.e., that we
     **  are not letting some nasty person use the setuid part
     **  of this program to look at or munge some presumably
     **  hidden files.
   */

  if (SccsDir[0] == '/' && !safepath (name))
    return (NULL);

  /*
     **  Create the base pathname.
   */

  /* first the directory part */
  if (SccsDir[0] != '\0' && name[0] != '/' && strncmp (name, "./", 2) != 0)
    {
      gstrcpy (buf, SccsDir, sizeof (buf));
      gstrcat (buf, "/", sizeof (buf));
    }
  else
    {
      gstrcpy (buf, "", sizeof (buf));
    }


  /* then the head of the pathname */
  gstrncat (buf, name, p - name, sizeof (buf)); /* will always be terminated */
  q = &buf[strlen (buf)];
  left = sizeof(buf) - strlen(buf);

  /* now copy the final part of the name, in case useful */
  gstrcpy (q, p, left);

  /* so is it useful? */
  if (strncmp (p, "s.", 2) != 0 && !isdir (buf))
    {
      /* sorry, no; copy the SCCS pathname & the "s." */
      gstrcpy (q, SccsPath, left);
      gstrcat (buf, "/s.", sizeof (buf));

      /* and now the end of the name */
      gstrcat (buf, p, sizeof (buf));
    }

  /* if I haven't changed it, why did I do all this? */
  /* but if I have, squirrel it away */
  /* So our actions are the same in either case... */
  return str_dup (buf);
}

/*
   **  ISDIR -- return true if the argument is a directory.
   **
   **   Parameters:
   **           name -- the pathname of the file to check.
   **
   **   Returns:
   **           TRUE if 'name' is a directory, FALSE otherwise.
   **
   **   Side Effects:
   **           none.
 */

bool
isdir (const char *name)
{
  struct stat stbuf;

#ifdef S_ISDIR
  return (stat (name, &stbuf) >= 0) && S_ISDIR(stbuf.st_mode);
#else
  return (stat (name, &stbuf) >= 0) && (stbuf.st_mode & S_IFDIR);
#endif
}

/*
   **  SAFEPATH -- determine whether a pathname is "safe"
   **
   **   "Safe" pathnames only allow you to get deeper into the
   **   directory structure, i.e., full pathnames and ".." are
   **   not allowed.
   **
   **   Parameters:
   **           p -- the name to check.
   **
   **   Returns:
   **           TRUE -- if the path is safe.
   **           FALSE -- if the path is not safe.
   **
   **   Side Effects:
   **           Prints a message if the path is not safe.
 */

bool
safepath (register const char *p)
{
  const char *arg = p;
  if (*p != '/')
    {
      while (strncmp (p, "../", 3) != 0 && strcmp (p, "..") != 0)
        {
          p = my_index (p, '/');
          if (p == NULL)
            return TRUE;
          p++;
        }
    }

  printf ("You may not use full pathnames or \"..\" but you specifed '%s'\n", arg);
  return FALSE;
}

static void
form_gname(char *buf, size_t bufsize, struct dirent *dir)
{
  size_t len = strlen (dir->d_name);
  size_t gname_len = len-2u;

  if (gname_len >= bufsize)
    {
      gstrbotchn(bufsize, dir->d_name, len, (char*)0, 0);
    }
  else
    {
      memcpy(buf, dir->d_name+2, gname_len);
      buf[gname_len] = 0; /* terminate the string. */
    }
}


/*
   **  CLEAN -- clean out recreatable files
   **
   **   Any file for which an "s." file exists but no "p." file
   **   exists in the current directory is purged.
   **
   **   Parameters:
   **           mode -- tells whether this came from a "clean", "info", or
   **                   "check" command.
   **           argv -- the rest of the argument vector.
   **
   **   Returns:
   **           none.
   **
   **   Side Effects:
   **           Removes files in the current directory.
   **           Prints information regarding files being edited.
   **           Exits if a "check" command.
 */
int
do_clean (int mode, char *const *argv, char buf[FBUFSIZ])
{
  struct dirent *dir;
  register DIR *dirp;
  char *bufend;
  register char *basefile;
  bool gotedit;
  bool gotpfent;
  FILE *pfp;
  bool nobranch = FALSE;
  register const struct pfile *pf;
  register char *const *ap;
  const char *usernm = NULL;
  const char *subdir = NULL;
  const char *cmdname;

  /*
     **  Process the argv
   */

  cmdname = *argv;
  for (ap = argv; *++ap != NULL;)
    {
      if (**ap == '-')
        {
          /* we have a flag */
          switch ((*ap)[1])
            {
            case 'b':
              nobranch = TRUE;
              break;

            case 'u':
              if ((*ap)[2] != '\0')
                usernm = &(*ap)[2];
              else if (ap[1] != NULL && ap[1][0] != '-')
                usernm = *++ap;
              else
                usernm = username ();
              break;
            }
        }
      else
        {
          if (subdir != NULL)
            usrerr ("too many args");
          else
            subdir = *ap;
        }
    }

  /*
     **  Find and open the SCCS directory.
   */

  gstrcpy (buf, SccsDir, FBUFSIZ);
  if (buf[0] != '\0')
    gstrcat (buf, "/", FBUFSIZ);
  if (subdir != NULL)
    {
      gstrcat (buf, subdir, FBUFSIZ);
      gstrcat (buf, "/", FBUFSIZ);
    }
  gstrcat (buf, SccsPath, FBUFSIZ);
  bufend = &buf[strlen (buf)];

  dirp = opendir (buf);
  if (dirp == NULL)
    {
      usrerr ("cannot open %s", buf);
      return (CSSC_EX_NOINPUT);
    }

  /*
     **  Scan the SCCS directory looking for s. files.
     **   gotedit tells whether we have tried to clean any
     **           files that are being edited.
   */

  gotedit = FALSE;
  while (NULL != (dir = readdir (dirp)))
    {
      if ('s' != dir->d_name[0] ||
	  '.' != dir->d_name[1] ||
	  0 == dir->d_name[2])
        continue;

      /* got an s. file -- see if the p. file exists */
      gstrcat (buf, "/p.", FBUFSIZ);/* XXX: BUG: wrong size limit. */
      basefile = bufend + 3;
      form_gname(basefile, FBUFSIZ-strlen(buf), dir);


      /*
         **  open and scan the p-file.
         **   'gotpfent' tells if we have found a valid p-file
         **           entry.
       */

      pfp = fopen (buf, "r");
      gotpfent = FALSE;
      if (pfp != NULL)
        {
          /* the file exists -- report it's contents */
          while ((pf = getpfent (pfp)) != NULL)
            {
              if (nobranch && isbranch (pf->p_nsid))
                continue;
              if (usernm != NULL && strcmp (usernm, pf->p_user) != 0 && mode != CLEANC)
                continue;
              gotedit = TRUE;
              gotpfent = TRUE;
              if (mode == TELLC)
                {
                  printf ("%s\n", basefile);
                  break;
                }
              printf ("%12s: being edited: ", basefile);
              putpfent (pf, stdout);
            }
          fclose (pfp);
        }

      /* the s. file exists and no p. file exists -- unlink the g-file */
      if (mode == CLEANC && !gotpfent)
        {
          char unlinkbuf[FBUFSIZ];
          form_gname(unlinkbuf, FBUFSIZ, dir);
          unlink (unlinkbuf);
        }
    }

  /* cleanup & report results */
  closedir (dirp);
  if (!gotedit && mode == INFOC)
    {
      printf ("Nothing being edited");
      if (nobranch)
        printf (" (on trunk)");
      if (usernm == NULL)
        printf ("\n");
      else
        printf (" by %s\n", usernm);
    }
  if (mode == CHECKC)
    exit (gotedit);
  return (CSSC_EX_OK);
}

int
clean (int mode, char *const *argv)
{
  int retval = CSSC_EX_OK;
  char *buf = malloc(FBUFSIZ);
  if (NULL == buf)
    {
      oom();
    }
  else
    {
      retval = do_clean(mode, argv, buf);
      free(buf);
    }
  return retval;
}

/*
   **  ISBRANCH -- is the SID a branch?
   **
   **   Parameters:
   **           sid -- the sid to check.
   **
   **   Returns:
   **           TRUE if the sid represents a branch.
   **           FALSE otherwise.
   **
   **   Side Effects:
   **           none.
 */

int
isbranch (const char *sid)
{
  register const char *p;
  int dots;

  dots = 0;
  for (p = sid; *p != '\0'; p++)
    {
      if (*p == '.')
        dots++;
      if (dots > 1)
        return TRUE;
    }
  return FALSE;
}

/*
   **  UNEDIT -- unedit a file
   **
   **   Checks to see that the current user is actually editting
   **   the file and arranges that s/he is not editting it.
   **
   **   Parameters:
   **           fn -- the name of the file to be unedited.
   **
   **   Returns:
   **           TRUE -- if the file was successfully unedited.
   **           FALSE -- if the file was not unedited for some
   **                   reason.
   **
   **   Side Effects:
   **           fn is removed
   **           entries are removed from pfile.
 */

bool
unedit (const char *fn)
{
  register FILE *pfp;
  const char *cp;
  char *pfn;
  FILE *tfp;
  register char *q;
  bool delete = FALSE;
  bool others = FALSE;
  const char *myname;
  const struct pfile *pent;
  char buf[PFILELG];

  /* make "s." filename & find the trailing component */
  pfn = makefile (fn);          /* returned value must be freed. */
  if (pfn == NULL)
    return (FALSE);
  q = my_rindex (pfn, '/');
  if (q == NULL)
    q = &pfn[-1];
  if (q[1] != 's' || q[2] != '.')
    {
      usrerr ("bad file name \"%s\"", fn);
      free(pfn);
      return (FALSE);
    }

  /* turn "s." into "p." & try to open it */
  *++q = 'p';

  pfp = fopen (pfn, "r");
  if (pfp == NULL)
    {
      printf ("%12s: not being edited\n", fn);
      free(pfn);
      return (FALSE);
    }

  /* create temp file for editing p-file */
  tfp = tmpfile();
  if (tfp == NULL)
    {
      usrerr ("cannot create temporary file");
      fclose(pfp);
      free(pfn);
      exit (CSSC_EX_OSERR);
    }

  /* figure out who I am */
  myname = username ();

  /*
   *  Copy p-file to temp file, doing deletions as needed.
   */

  while ((pent = getpfent (pfp)) != NULL)
    {
      if (strcmp (pent->p_user, myname) == 0)
        {
          /* a match */
          delete = TRUE;
        }
      else
        {
          /* output it again */
          putpfent (pent, tfp);
          others++;
        }
    }

  /*
   * Before changing anything, make sure we can remove
   * the file in question (assuming it exists).
   */
  if (delete)
    {
      extern int errno;

      cp = tail (pfn);
      if ('p' == cp[0] && '.' == cp[1])
	{
	  cp += 2;
	}
      errno = 0;
      if (access (cp, 0) < 0 && errno != ENOENT)
        goto bad;
      if (errno == 0)
        /*
         * This is wrong, but the rest of the program
         * has built in assumptions about "." as well,
         * so why make unedit a special case?
         */
        if (access (".", 2) < 0)
          {
          bad:
            printf ("%12s: can't remove\n", cp);
            fclose (tfp);
            fclose (pfp);
            free(pfn);
            return (FALSE);
          }
    }
  /* do final cleanup */
  if (others)
    {
      /* copy it back (perhaps it should be linked?) */
      rewind(tfp);

      if (freopen (pfn, "w", pfp) == NULL)
        {
          usrerr ("cannot create \"%s\"", pfn);
          free(pfn);
          return (FALSE);
        }
      while (fgets (buf, sizeof buf, tfp) != NULL)
        fputs (buf, pfp);
    }
  else
    {
      /* it's empty -- remove it */
      unlink (pfn);
    }
  fclose (tfp);
  fclose (pfp);

  /* actually remove the g-file */

  /* TODO: %12s in the printfs below is inappropriate for modern Unix
   * where filenames can be longer than 11 characters...
   */

  if (delete)
    {
      /*
       * Since we've checked above, we can
       * use the return from unlink to
       * determine if the file existed or not.
       */
      if (unlink (cp) >= 0)
        printf ("%12s: removed\n", cp);
      free(pfn);
      return (TRUE);
    }
  else
    {
      printf ("%12s: not being edited by you\n", fn);
      free(pfn);
      return (FALSE);
    }
}

/*
   **  DODIFF -- diff an s-file against a g-file
   **
   **   Parameters:
   **           getv -- argv for the 'get' command.
   **           gfile -- name of the g-file to diff against.
   **
   **   Returns:
   **           Result of get.
   **
   **   Side Effects:
   **           none.
 */
int
dodiff (char * getv[], const char *gfile)
{
  int pipev[2];
  int rval;
  register int pid;
  auto int st;
  extern int errno;

  printf ("\n------- %s -------\n", gfile);
  fflush (stdout);

  /* create context for diff to run in */
  if (pipe (pipev) < 0)
    {
      syserr ("dodiff: pipe failed");
      exit (CSSC_EX_OSERR);
    }
  if ((pid = do_fork ()) < 0)
    {
      syserr ("dodiff: fork failed");
      exit (CSSC_EX_OSERR);
    }
  else if (pid > 0)
    {
      /* in parent; run get */
      OutFile = pipev[1];
      close (pipev[0]);
      rval = command (&getv[1], TRUE, "get:rcixt -s -k -p");

      childwait(pid, &st, 1);   /* ignore SIGINT while waiting. */
      /* ignore result of diff */
    }
  else
    {
      /* in child, run diff */
      if (close (pipev[1]) < 0 || close (0) < 0 ||
          dup (pipev[0]) != 0 || close (pipev[0]) < 0)
        {
          syserr ("dodiff: failed to set up the 'diff' end of the pipe");
          exit (CSSC_EX_OSERR);
        }
      /* The aBdHpqsvy options are (usually) specific to GNU diff. */
      /* GNU diff supports -v (show version) but I don't think
       *  this is useful.  Nevertheless we pass it through.
       */
      command (&getv[1], FALSE, "-diff:elsfhbCunaBdHpqsvwyD");
    }
  return rval;
}

/*
   **  TAIL -- return tail of filename.
   **
   **   Parameters:
   **           fn -- the filename.
   **
   **   Returns:
   **           a pointer to the tail of the filename; e.g., given
   **           "cmd/ls.c", "ls.c" is returned.
   **
   **   Side Effects:
   **           none.
 */

const char *
tail (register const char *fn)
{
  register const char *p;

  for (p = fn; *p != 0; p++)
    if (*p == '/' && p[1] != '\0' && p[1] != '/')
      fn = &p[1];
  return fn;
}

/*
   **  TAIL_NC -- return tail of filename (non-const version).
   **
   **   Parameters:
   **           fn -- the filename.
   **
   **   Returns:
   **           a pointer to the tail of the filename; e.g., given
   **           "cmd/ls.c", "ls.c" is returned.
   **
   **   Side Effects:
   **           none.
 */

char *
tail_nc (register char *fn)
{
  register char *p;

  for (p = fn; *p != 0; p++)
    if (*p == '/' && p[1] != '\0' && p[1] != '/')
      fn = &p[1];
  return fn;
}

/*
   **  GETPFENT -- get an entry from the p-file
   **
   **   Parameters:
   **           pfp -- p-file file pointer
   **
   **   Returns:
   **           pointer to p-file struct for next entry
   **           NULL on EOF or error
   **
   **   Side Effects:
   **           Each call wipes out results of previous call.
 */

const struct pfile *
getpfent (FILE * pfp)
{
  static struct pfile ent;
  static char buf[PFILELG];
  register char *p;

  if (fgets (buf, sizeof buf, pfp) == NULL)
    return NULL;

  ent.p_osid = p = buf;
  ent.p_nsid = p = nextfield (p);
  ent.p_user = p = nextfield (p);
  ent.p_date = p = nextfield (p);
  ent.p_time = p = nextfield (p);
  ent.p_aux = p = nextfield (p);

  return &ent;
}


char *
nextfield (register char *p)
{
  if (p == NULL || *p == '\0')
    return NULL;

  while (*p != ' ' && *p != '\n' && *p != '\0')
    p++;

  if (*p == '\n' || *p == '\0')
    {
      *p = '\0';
      return NULL;
    }
  *p++ = '\0';
  return p;
}
/*
   **  PUTPFENT -- output a p-file entry to a file
   **
   **   Parameters:
   **           pf -- the p-file entry
   **           f -- the file to put it on.
   **
   **   Returns:
   **           none.
   **
   **   Side Effects:
   **           pf is written onto file f.
 */

void
putpfent (register const struct pfile *pf, register FILE * f)
{
  fprintf (f, "%s %s %s %s %s", pf->p_osid, pf->p_nsid,
           pf->p_user, pf->p_date, pf->p_time);

  if (pf->p_aux != NULL)
    fprintf (f, " %s", pf->p_aux);
  else
    fprintf (f, "\n");
}

/*
   **  USRERR -- issue user-level error
   **
   **   Parameters:
   **           f -- format string.
   **           p1-p3 -- parameters to a printf.
   **
   **   Returns:
   **           -1
   **
   **   Side Effects:
   **           none.
 */

void
usrerr (const char *fmt,...)
{
  va_list ap;


  fprintf (stderr, "\n%s: ", program_name);

  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);

  fprintf (stderr, "\n");
}

/*
   **  SYSERR -- print system-generated error.
   **
   **   Parameters:
   **           f -- format string to a printf.
   **           p1, p2, p3 -- parameters to f.
   **
   **   Returns:
   **           never.
   **
   **   Side Effects:
   **           none.
 */


void
syserr (const char *fmt,...)
{
  extern int errno;
  va_list ap;

  fprintf (stderr, "\n%s SYSERR: ", program_name);

  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);

  fprintf (stderr, "\n");

  if (errno == 0)
    {
      exit (CSSC_EX_SOFTWARE);
    }
  else
    {
      perror (NULL);
      exit (CSSC_EX_OSERR);
    }
}
/*
   **  USERNAME -- return name of the current user
   **
   **   Parameters:
   **           none
   **
   **   Returns:
   **           name of current user
   **
   **   Side Effects:
   **           none
 */

const char *
username (void)
{
  const struct passwd *pw;

  pw = getpwuid (getuid ());
  if (pw == NULL)
    {
      syserr ("Who are you?\n"
              "You don't seem to have an entry in the user database "
              "(/etc/passwd) (uid=%d)", (int)getuid ());
      exit (CSSC_EX_OSERR);
    }
  return (pw->pw_name);
}

/*
   **   Guarded string manipulation routines; the last argument
   **   is the length of the buffer into which the strcpy or strcat
   **   is to be done.
 */
static char *
gstrcat (char *to, const char *from, size_t length)
{
  if (strlen (from) + strlen (to) >= length)
    {
      gstrbotch (to, from);
    }
  return (strcat (to, from));
}

static
char *
gstrncat (char *to, const char *from, size_t n, size_t length)
{
  if (n + strlen (to) >= length)
    {
      gstrbotch (to, from);
    }
  /* strncat() always appends a \0 to the destination. */
  return strncat (to, from, n);
}

static char *
gstrcpy (char *to, const char *from, size_t length)
{
  if (strlen (from) >= length)
    {
      gstrbotch (from, (char *) 0);
    }
  return strcpy (to, from);
}

static void
gstrbotch (const char *str1, const char *str2)
{
  usrerr ("Filename(s) too long: %s %s",
          (str1 ? str1 : ""),
          (str2 ? str2 : ""));
  exit(CSSC_EX_SOFTWARE);
}

static void
gstrbotchn (int navail,
            const char *str1, int len1, const char *str2, int len2)
{
  fprintf(stderr, "Filename%s too long: ", (str1 && str2) ? "s" :"");
  if (str1)
    {
      fwrite(str1, len1, 1, stderr);
      putc(' ', stderr);
    }
  if (str2)
    {
      fwrite(str2, len2, 1, stderr);
    }
  putc('\n', stderr);
  fprintf(stderr, "Only %d characters available.\n", navail);
  exit(CSSC_EX_SOFTWARE);
}
