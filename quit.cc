/*
 * quit.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Functions for cleaning up and quitting.
 *
 */

#ifdef __GNUC__
#pragma implementation "quit.h"
#endif

#include "cssc.h"
#include "sysdep.h"
#include "quit.h"

#include <stdarg.h>

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: quit.cc,v 1.5 1997/05/10 14:49:53 james Exp $";
#endif

#ifdef CONFIG_BORLANDC
#ifdef __STDC__
extern "C" char * CDECL strlwr(char *);
#endif
#include <dir.h>
#endif /* CONFIG_BORLANDC */

const char *prg_name = NULL;

void
set_prg_name(const char *name) {
#ifdef CONFIG_BORLANDC

	static char file[MAXFILE + MAXEXT];
	char ext[MAXEXT];

	fnsplit(name, NULL, NULL, file, ext);
	strlwr(file);
	strlwr(ext);

	if (strcmp(ext, ".exe") != 0 && strcmp(ext, ".com") != 0) {
		strcat(file, ext);
	}
	prg_name = file;

#else /* CONFIG_BORLANDC */

	prg_name = name;

#endif /* CONFIG_BORLANDC */
}

NORETURN
quit(int err, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);

	fflush(stdout);
	cleanup::run_cleanups();

	fflush(stdout);

	putc('\n', stderr);

	if (err == -2) {
		usage();
	}


	if (prg_name != NULL && err != 0) {
		fprintf(stderr, "%s: ", (const char *) prg_name);
	}

	vfprintf(stderr, fmt, ap);

	va_end(ap);

	putc('\n', stderr);

	if (err >= 1) {
#ifndef HAVE_STRERROR
		if (err <= sys_nerr) {
			fprintf(stderr, "%d - %s\n", err, sys_errlist[err]);
		} else {
			fprintf(stderr, "%d - Unknown error\n", err);
		}
#else
		fprintf(stderr, "%d - %s\n", err, strerror(err));
#endif
	}

	fflush(stderr);

#ifdef HAVE_ABORT
	if (err == -3) {
		abort();
	}
#endif

#if HAVE_PIPE
	if (cleanup::in_child()) {
		if (err > 0) {
			_exit(1);
		} else {
			_exit(-err);
		}
	}
#endif

	if (err > 0) {
		exit(1);
	} else {
		exit(-err);
	}
}

NORETURN
nomem() {
	quit(-4, "Out of memory!");
}

NORETURN
assert_failed(const char *file, int line, const char *test) {
	quit(-3, "Assertion failed: %s\nFile: %s  Line: %d", test, file, line);
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

cleanup::~cleanup() {
	class cleanup *p = head;

	if (p == this) {
		head = next;
		return;
	}

	while(p->next != this) {
		p = p->next;
		assert(p != NULL);
	}

	p->next = next;
}		

void
cleanup::run_cleanups() {
	if (running || all_disabled) {
		return;
	}
	running = 1;

	class cleanup *p = head;
	while(p != NULL) {
		p->do_cleanup();
		p = p->next;
	}

	running = 0;
	all_disabled++;
	return;
}

/* Local variables: */
/* mode: c++ */
/* End: */
