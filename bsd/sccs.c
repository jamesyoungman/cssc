/*
 * Copyright (C) 1998
 *      Free Software Foundation, Inc.  All rights reserved.
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
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

#define _BSD_SOURCE
#define DEBUG (1)
 

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1980, 1993\n"
"The Regents of the University of California.  All rights reserved.\n"
"@(#) Copyright (c) 1998\n"
"Free Software Foundation, Inc.  All rights reserved.\n";
#endif /* not lint */
static const char filever[] = "$Id: sccs.c,v 1.12 1998/06/06 13:35:55 james Exp $";

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#ifdef STDC_HEADERS
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/types.h>

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

/* Cope with systems that have no (compatible) sys/wait.h.
 */
#ifndef WEXITSTATUS
#define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
#define WIFEXITED(stat_val) (((stat_val) & 0377) == 0)
#endif
#ifndef WTERMSIG
#define WTERMSIG(stat_val) ( (stat_val) & 0177 )
#endif
#ifndef WCOREDUMP
#define WCOREDUMP(stat_val) ( (stat_val) & 0200 )
#endif

#include <sys/cdefs.h>		/* TODO: this does what? */
#include <sys/param.h>		/* TODO: this does what? */


#include <sys/stat.h>

#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif




#include <signal.h>		/* TODO: consider using sigaction(). */
#include <errno.h>		/* TODO: same as in parent directory. */
#include <pwd.h>		/* getpwuid() */

#include <sysexits.h>		/* TODO: we should probably define our own. */

/* #include "pathnames.h" */
/* The next few lines inserted from @(#)pathnames.h	8.1 (Berkeley) 6/6/93
 */
#include <paths.h>

/* #define PASTE(a,b) a##b */

#define	_PATH_SCCSADMIN	("admin")
#define	_PATH_SCCSBDIFF	("bdiff")
#define	_PATH_SCCSCOMB	("comb")
#define	_PATH_SCCSDELTA	("delta")
#define	_PATH_SCCSDIFF	("sccsdiff")
#define	_PATH_SCCSGET	("get")
#define	_PATH_SCCSHELP	("help")
#define	_PATH_SCCSPRS	("prs")
#define	_PATH_SCCSPRT	("prt")
#define	_PATH_SCCSRMDEL	("rmdel")
#define	_PATH_SCCSVAL	("val")
#define	_PATH_SCCSWHAT	("what")
#undef _PATH_TMP
#define	_PATH_TMP	"/tmp/sccsXXXXX"

/* End of insertion from pathnames.h */


#ifndef PREFIX
#define PREFIX "/usr/sccs/"
#endif


#ifdef STAT_MACROS_BROKEN
#undef S_ISDIR
#endif


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
   **   Commands:
   **           admin,
   **           get,
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
   **           SCCSPATH -- the default for the -p flag.
   **           MYNAME -- the title this program should print when it
   **                   gives error messages.
   **
   **           UIDUSER -- removed since we cannot trust $USER
   **                      or getlogin in setuid programs.
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
#define SCCSPATH	"SCCS"	/* pathname in which to find s-files */
#endif

#ifndef MYNAME
#define MYNAME		"sccs"	/* name used for printing errors */
#endif

/****************  End of Configuration Information  ****************/

typedef char bool;
#define TRUE	1
#define FALSE	0

#define bitset(bit, word)	((bool) ((bit) & (word)))

struct sccsprog
  {
    const char *sccsname;	/* name of SCCS routine */
    short sccsoper;		/* opcode, see below */
    short sccsflags;		/* flags, see below */
    const char *sccspath;	/* pathname of binary implementing */
  };

/* values for sccsoper */
#define PROG		0	/* call a program */
#define CMACRO		1	/* command substitution macro */
#define FIX		2	/* fix a delta */
#define CLEAN		3	/* clean out recreatable files */
#define UNEDIT		4	/* unedit a file */
#define SHELL		5	/* call a shell file (like PROG) */
#define DIFFS		6	/* diff between sccs & file out */
#define DODIFF		7	/* internal call to diff program */
#define ENTER		8	/* enter new files */

/* bits for sccsflags */
#define NO_SDOT	0001		/* no s. on front of args */
#define REALUSER	0002	/* protected (e.g., admin) */

/* modes for the "clean", "info", "check" ops */
#define CLEANC		0	/* clean command */
#define INFOC		1	/* info command */
#define CHECKC		2	/* check command */
#define TELLC		3	/* give list of files being edited */

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
  {"admin", PROG, REALUSER, _PATH_SCCSADMIN},
  {"cdc", PROG, 0, _PATH_SCCSRMDEL},
  {"comb", PROG, 0, _PATH_SCCSCOMB},
  {"delta", PROG, 0, _PATH_SCCSDELTA},
  {"get", PROG, 0, _PATH_SCCSGET},
  {"help", PROG, NO_SDOT, _PATH_SCCSHELP},
  {"prs", PROG, 0, _PATH_SCCSPRS},
  {"prt", PROG, 0, _PATH_SCCSPRT},
  {"rmdel", PROG, REALUSER, _PATH_SCCSRMDEL},
  {"val", PROG, 0, _PATH_SCCSVAL},
  {"what", PROG, NO_SDOT, _PATH_SCCSWHAT},
  {"sccsdiff", SHELL, REALUSER, _PATH_SCCSDIFF},
  {"edit", CMACRO, NO_SDOT, "get -e"},
  {"delget", CMACRO, NO_SDOT, "delta:mysrp/get:ixbeskcl -t"},
  {"deledit", CMACRO, NO_SDOT, "delta:mysrp -n/get:ixbskcl -e -t -g"},
  {"fix", FIX, NO_SDOT, NULL},
  {"clean", CLEAN, REALUSER | NO_SDOT, (char *) CLEANC},
  {"info", CLEAN, REALUSER | NO_SDOT, (char *) INFOC},
  {"check", CLEAN, REALUSER | NO_SDOT, (char *) CHECKC},
  {"tell", CLEAN, REALUSER | NO_SDOT, (char *) TELLC},
  {"unedit", UNEDIT, NO_SDOT, NULL},
  {"diffs", DIFFS, NO_SDOT | REALUSER, NULL},
  {"-diff", DODIFF, NO_SDOT | REALUSER, _PATH_SCCSBDIFF},
  {"print", CMACRO, 0, "prs -e/get -p -m -s"},
  {"branch", CMACRO, NO_SDOT, "get:ixrc -e -b/delta: -s -n -ybranch-place-holder/get:pl -e -t -g"},
  {"enter", ENTER, NO_SDOT, NULL},
  {"create", CMACRO, NO_SDOT, "enter/get:ixbeskcl -t"},
  {NULL, -1, 0, NULL},
};

/* one line from a p-file */
struct pfile
  {
    char *p_osid;		/* old SID */
    char *p_nsid;		/* new SID */
    char *p_user;		/* user who did edit */
    char *p_date;		/* date of get */
    char *p_time;		/* time of get */
    char *p_aux;		/* extra info at end */
  };

const char *SccsPath = SCCSPATH;	/* pathname of SCCS files */
#ifdef SCCSDIR
const char *SccsDir = SCCSDIR;	/* directory to begin search from */
#else
const char *SccsDir = "";
#endif
char *subprogram_exec_prefix = NULL; /* see try_to_exec(). */

const char MyName[] = MYNAME;	/* name used in messages */
int OutFile = -1;		/* override output file for commands */
bool RealUser;			/* if set, running as real user */
#ifdef DEBUG
bool Debug = 0;			/* turn on tracing */
#endif
static bool TrustEnvironment = 0;


void syserr (const char *fmt,...);
void usrerr (const char *fmt,...);
int command (char *argv[], bool forkflag, const char *arg0);
int callprog (const char *progpath, short flags,
	      char *const argv[], bool forkflag);
int clean (int mode, char *const argv[]);
int dodiff (char *const getv[], const char *gfile);
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


static char *gstrcat (char *to, const char *from, int length);
static char *gstrncat (char *to, const char *from, int n, int length);
static char *gstrcpy (char *to, const char *from, int length);
static void gstrbotch (const char *str1, const char *str2);
static void  gstrbotchn (int avail, const char *, int, const char *, int);

static char *str_dup (const char *);
static void childwait(int pid, int *status_ptr, int ignoreintr);


/* #define	FBUFSIZ	BUFSIZ */

#define FBUFSIZ 1024

static void
show_version(void)
{
  fprintf(stderr, "%s from GNU CSSC %s\n%s\n", MyName, (VERSION), filever);
  fprintf(stderr, "SccsPath = '%s'\nSccsDir = '%s'\n", SccsPath, SccsDir);
  fprintf(stderr, "Default prefix for SCCS subcommands is '%s'\n", (PREFIX));
}


static void oom(void)
{
  perror("malloc failed");
  exit(EX_TEMPFAIL);
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
	  if (subprogram_exec_prefix)
	    free(subprogram_exec_prefix);
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
	      "Option --prefix is incompatible with setuid "
	      "execution.  Sorry.\n");
      exit (EX_USAGE);
    }
}

static void 
usage(void)
{
  fprintf (stderr, "Usage: %s [flags] command [flags]\n", MyName);
}

#define	PFILELG	120

int 
main (int argc, char **argv)
{
  register char *p;
  register int i;
  int hadver = 0;
  
  &copyright;			/* prevent warning about unused variable. */

  if (getuid() != geteuid())
    TrustEnvironment = 0;	/* running setuid, ignore $PATH etc. */
  else
    TrustEnvironment = 1;

  if (TrustEnvironment)
    {
      if (NULL == setlocale(LC_ALL, ""))
	{
	  /* If we can't set the locale as the user wishes,
	   * emit an error message and continue.   The error
	   * message will of course be in the "C" locale.
	   */
	  perror("Error setting locale");
	}
    }

  /* If we do not trust the environment, should we trust PROJECTDIR?
   * I think yes, because the setuid mechanism may be being used
   * to conrol access to source.
   */
#ifndef SCCSDIR
  
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
	      exit(EX_USAGE);
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
		  exit(EX_USAGE);
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
      exit (EX_USAGE);
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
	      if (0 == *p)	/* just "--" */
		{
		  fprintf(stderr,
			  "End-of-arguments option -- not "
			  "supported, sorry.\n");
		  exit (EX_USAGE);
		}
	      else if (0 == strncmp(p, "prefix=", 7))
		{
		  set_prefix(p+7);
		}
	      else if (0 == strcmp(p, "cssc"))
		{
		  printf("%s\n", "yes");
		  exit(EX_OK);
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
		  exit (EX_USAGE);
		}
	      break;
	      
	    case 'V':
	      if (!hadver)
		show_version();
	      hadver = 1;
	      if (2 == argc)	/* If -V was the only arg, return success. */
		return 0;
	      break;		/* Otherwise, process the remaining options. */
	      
	    case 'r':		/* run as real user */
	      setuid (getuid ());
	      RealUser++;
	      break;

#ifndef SCCSDIR
	    case 'p':		/* path of sccs files */
	      SccsPath = ++p;
	      if (SccsPath[0] == '\0' && argv[1] != NULL)
		SccsPath = *++argv;
	      break;

	    case 'd':		/* directory to search from */
	      SccsDir = ++p;
	      if (SccsDir[0] == '\0' && argv[1] != NULL)
		SccsDir = *++argv;
	      break;
#else
	    case 'p':
	    case 'd':
	      fprintf(stderr, "The %c option has been disabled.\n", *p);
	      exit(EX_USAGE);
	      break;
#endif

	    case 'T':		/* trace */
#ifdef DEBUG
	      Debug++;
#else
	      fprintf(stderr, "The -T option has been disabled.  Sorry.\n");
	      exit(EX_USAGE);
#endif
	      break;

	    default:
	      usrerr ("unknown option -%s", p);
	      usage();
	      exit (EX_USAGE);
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
	}
      else
	{
	  /* Just "sccs -V" is valid. */
	  exit(0);
	}
    }
  else
    {
      i = command (argv, FALSE, "");
      exit (i);
    }
}

/* Warning in do_enter*/
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
      ap[0] = buf2;	/* sccs enter foo --> admin -ifoo */
      argv[0] = tail_nc (np[0]);	/* ?Warning: discards const */
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
	execvp(prog, argv);		/* execvp() uses $PATH */
    }
  
  /* absolute_pathname() does not call a library function, so no need
   * to save/restore errno.
   */
  if (absolute_pathname(prog))	
    {
      perror(prog);
    }
  else
    {
      /* if the above exec() returns or was not allowed, try our prefix.
       */
      len = strlen(prefix) + strlen(prefix);
      newprog = malloc(len + 1);
      if (NULL == newprog)
	{
	  oom();
	  /*NOTREACHED*/
	  exit(EX_TEMPFAIL);
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
  int rval = 0;			/* value to be returned. */

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
  if (1)			/* introduce scope for editchs. */
    {
      char *editchs;
      const char *p;
      
      editchs = NULL;
      if (1)			/* introduce scope */
	{
	  char *q;
      
	  for (p = arg0, q = buf; *p != '\0' && *p != '/';)
	    {
	      *np++ = q;
	      while (*p == ' ')		/* wind p to next word. */
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
	  return (EX_USAGE);
	}

      /*
      **  Copy remaining arguments doing editing as appropriate.
      */

      for (; *argv != NULL; argv++)
	{
	  p = *argv;
	  if (*p == '-')
	    {
	      if (p[1] == '\0' || editchs == NULL || index (editchs, p[1]) != NULL)
		*np++ = p;	/* Warning: discards const */
	    }
	  else
	    {
	      if (!bitset (NO_SDOT, cmd->sccsflags))
		p = makefile (p);	/* MEMORY LEAK (of returned value) */
	      if (p != NULL)
		*np++ = p;	/* Warning: discards const */
	    }
	}
      *np = NULL;
    }
  
  /*
     **  Interpret operation associated with this command.
   */

  switch (cmd->sccsoper)
    {
    case SHELL:		/* call a shell file */
      {
	ap[0]  = cmd->sccspath;	/* Warning: discards const */
	ap[-1] = "sh";
	rval = callprog (_PATH_BSHELL, cmd->sccsflags, ap, forkflag);
      }
      break;

    case PROG:			/* call an sccs prog */
      {
	rval = callprog (cmd->sccspath, cmd->sccsflags, ap, forkflag);
      }
      break;

    case CMACRO:		/* command macro */
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

    case FIX:			/* fix a delta */
      {
	if (ap[1] == 0 || strncmp (ap[1], "-r", 2) != 0)
	  {
	    usrerr ("-r flag needed for fix command");
	    rval = EX_USAGE;
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
	rval = clean ((int) cmd->sccspath, ap);
      }
      break;

    case UNEDIT:
      {
	for (argv = np = &ap[1]; *argv != NULL; argv++)
	  {
	    if (unedit (*argv))
	      *np++ = *argv;
	  }
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

    case DIFFS:		/* diff between s-file & edit file */
      {
	const char *s;
	
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
	    argv[1] = s;	/* Warning: discards const */
	  }
      }
      break;

    case DODIFF:		/* internal diff call */
      {
	setuid (getuid ());
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
	exit (EX_OSERR);
      }
      /*NOTREACHED */
      break;

    case ENTER:		/* enter new sccs files */
      np = do_enter(argv, np, ap, &rval);
      break;

    default:
      {
	syserr ("oper %d", cmd->sccsoper);
	exit (EX_SOFTWARE);
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
  struct sigaction sa={0}, osa;
  int ret;

  /* temporarily ignore SIG_INT.
   */
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
  if (sig > 999) sig = 999;	/* prevent buffer overflow (!) */
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
      printf ("callprog:\n");
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
	printf ("Forking\n");
#endif
      i = do_fork ();
      if (i < 0)
	{
	  syserr ("cannot fork");
	  exit (EX_OSERR);
	}
      else if (i > 0)		/* parent */
	{
	  int st;
	  
	  childwait(i, &st, 0);	/* don't block SIGINT. */

	  if (WIFEXITED(st))	/* normal exit. */
	    {
	      st = WEXITSTATUS(st);
	    }
	  else			/* child exited via signal */
	    {
	      int sigcode = WTERMSIG(st);
	      if (sigcode != SIGINT && sigcode != SIGPIPE)
		{
		  char sigmsgbuf[11];
		  fprintf (stderr,
			   "sccs: %s: %s%s",
			   argv[0],
			   get_sig_name(sigcode, sigmsgbuf),
			   (WCOREDUMP(st) ? " (core dumped)" : "") );
		}
	      st = EX_SOFTWARE;
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
    {
      syserr ("callprog: setting stdout w/o forking");
      exit (EX_SOFTWARE);
    }

  /* 
   * in child (or didn't fork at all).
   */

  /* set protection as appropriate */
  if (bitset (REALUSER, flags))
    {
      setuid (getuid ());
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
  exit (EX_UNAVAILABLE);
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
  size_t len = strlen (s) + 1u;	/* include space for terminating '\0' */
  p = malloc (len);
  if (p)
    {
      memcpy (p, s, len);
    }
  else
    {
      perror ("Sccs: no mem");
      exit (EX_OSERR);
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
  int left;
  
  p = rindex (name, '/');
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
  if (*p != '/')
    {
      while (strncmp (p, "../", 3) != 0 && strcmp (p, "..") != 0)
	{
	  p = index (p, '/');
	  if (p == NULL)
	    return TRUE;
	  p++;
	}
    }

  printf ("You may not use full pathnames or \"..\"\n");
  return FALSE;
}
  
static void
form_gname(char *buf, int bufsize, struct dirent *dir)
{
  if (NAMLEN(dir)-2 >= bufsize)
    {
      gstrbotchn(bufsize, dir->d_name, NAMLEN(dir), (char*)0, 0);
    }
  else
    {
      memcpy(buf, dir->d_name+2, NAMLEN(dir)-2);
      buf[NAMLEN(dir)-2] = 0; /* terminate the string. */
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
      return (EX_NOINPUT);
    }

  /*
     **  Scan the SCCS directory looking for s. files.
     **   gotedit tells whether we have tried to clean any
     **           files that are being edited.
   */

  gotedit = FALSE;
  while (NULL != (dir = readdir (dirp)))
    {
      if (NAMLEN(dir) < 2 || 's' != dir->d_name[0] || '.' != dir->d_name[1])
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
  return (EX_OK);
}

int 
clean (int mode, char *const *argv)
{
  int retval = EX_OK;
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
  static char tfn[] = _PATH_TMP;
  FILE *tfp;
  register char *q;
  bool delete = FALSE;
  bool others = FALSE;
  const char *myname;
  const struct pfile *pent;
  char buf[PFILELG];

  /* make "s." filename & find the trailing component */
  pfn = makefile (fn);		/* returned value must be freed. */
  if (pfn == NULL)
    return (FALSE);
  q = rindex (pfn, '/');
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
  mktemp (tfn);
  tfp = fopen (tfn, "w");
  if (tfp == NULL)
    {
      usrerr ("cannot create \"%s\"", tfn);
      fclose(pfp);
      free(pfn);
      exit (EX_OSERR);
    }

  /* figure out who I am */
  myname = username ();

  /*
     **  Copy p-file to temp file, doing deletions as needed.
   */

  while ((pent = getpfent (pfp)) != NULL)
    {
      if (strcmp (pent->p_user, myname) == 0)
	{
	  /* a match */
	  delete++;
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

      cp = tail (fn);
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
	    unlink (tfn);
	    free(pfn);
	    return (FALSE);
	  }
    }
  /* do final cleanup */
  if (others)
    {
      /* copy it back (perhaps it should be linked?) */
      if (freopen (tfn, "r", tfp) == NULL)
	{
	  syserr ("cannot reopen \"%s\"", tfn);
	  free(pfn);
	  exit (EX_OSERR);
	}
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
  unlink (tfn);

  /* actually remove the g-file */
  
  /* TODO: %12s in the prontfs below is inappropriate
   * for modern Unix were filenames can be longer than 11 characters...
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
/* Warning in dodiff */
int 
dodiff (char *const getv[], const char *gfile)
{
  int pipev[2];
  int rval;
  register int i;
  register int pid;
  auto int st;
  extern int errno;

  printf ("\n------- %s -------\n", gfile);
  fflush (stdout);

  /* create context for diff to run in */
  if (pipe (pipev) < 0)
    {
      syserr ("dodiff: pipe failed");
      exit (EX_OSERR);
    }
  if ((pid = do_fork ()) < 0)
    {
      syserr ("dodiff: fork failed");
      exit (EX_OSERR);
    }
  else if (pid > 0)
    {
      /* in parent; run get */
      OutFile = pipev[1];
      close (pipev[0]);
      rval = command (&getv[1], TRUE, "get:rcixt -s -k -p"); /* Warning */

      childwait(pid, &st, 1);	/* ignore SIGINT while waiting. */
      /* ignore result of diff */
    }
  else
    {
      /* in child, run diff */
      if (close (pipev[1]) < 0 || close (0) < 0 ||
	  dup (pipev[0]) != 0 || close (pipev[0]) < 0)
	{
	  syserr ("dodiff: magic failed");
	  exit (EX_OSERR);
	}
      command (&getv[1], FALSE, "-diff:elsfhbC"); /* Warning: discards const */
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


  fprintf (stderr, "\n%s: ", MyName);

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

  fprintf (stderr, "\n%s SYSERR: ", MyName);

  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);

  fprintf (stderr, "\n");

  if (errno == 0)
    {
      exit (EX_SOFTWARE);
    }
  else
    {
      perror (NULL);
      exit (EX_OSERR);
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
	      "(/etc/passwd) (uid=%d)", getuid ());
      exit (EX_OSERR);
    }
  return (pw->pw_name);
}

/*
   **   Guarded string manipulation routines; the last argument
   **   is the length of the buffer into which the strcpy or strcat
   **   is to be done.
 */
static char *
gstrcat (char *to, const char *from, int length)
{
  if (strlen (from) + strlen (to) >= length)
    {
      gstrbotch (to, from);
    }
  return (strcat (to, from));
}

static
char *
gstrncat (char *to, const char *from, int n, int length)
{
  if (n + strlen (to) >= length)
    {
      gstrbotch (to, from);
    }
  /* strncat(), unlike strncpy(), always appends a \0 to the destination. */
  return strncat (to, from, n);
}

static char *
gstrcpy (char *to, const char *from, int length)
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
  exit(EX_SOFTWARE);
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
  exit(EX_SOFTWARE);
}

