/*
 * run.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Routines for running programmes.
 *
 */

#include "mysc.h"
#include "run.h"
#include "list.h"
#include "sysdep.h"

#ifdef CONFIG_SCCS_IDS
static const char sccs_id[] = "@(#) MySC run.c 1.1 93/11/09 17:17:58";
#endif

/* Runs a programme and returns its exit status. */

int
run(const char *prg, list<const char *> const &args) {
	int i;
	int len = args.length();

#if defined(CONFIG_NO_FORK) && defined(CONFIG_NO_SPAWN)

	int cmdlen = strlen(prg) + 1;
	
	for(i = 0; i < len; i++) {
		cmdlen += strlen(args[i]) + 1;
	}

	char *s = (char *) xmalloc(cmdlen + 1);

	strcpy(s, prg);
	
	for(i = 0; i < len; i++) {
		strcat(s, " ");
		strcat(s, args[i]);
	}

	int ret = system(s);

#ifdef CONFIG_DJGPP
	if (ret == -1) {
#else
	if (ret != 0) {
#endif
		quit(errno, "system(\"%s\") failed.", s);
	}

	free(s);

#else /* defined(CONFIG_NO_FORK) && defined(CONFIG_NO_SPAWN) */

	const char *  *argv = (const char *  *) xmalloc((len + 2) 
						      * sizeof(const char *  *));

	argv[0] = prg;

	for(i = 0; i < len; i++) {
		argv[i + 1] = args[i];
	}
	
	argv[i + 1] = NULL;

#ifdef CONFIG_NO_FORK

	int ret = spawnvp(P_WAIT, (char *) prg, (char **) argv);
	if (ret == -1) {
		quit(errno, "spawnvp(\"%s\") failed.", prg);
	}

#else /* CONFIG_NO_FORK */

	int pid = fork(); 
	if (pid < 0) {
		quit(errno, "fork() failed.");
	}

	if (pid == 0) {
		cleanup::set_in_child();
		execvp(prg, (char **) argv);
		quit(errno, "execvp(\"%s\") failed.", prg);
	}

	int ret;
	int r = wait(&ret);
	while(r != pid) {
		if (r == -1 && errno != EINTR) {
			quit(errno, "wait() failed.");
		}
		r = wait(&ret);
	}

#endif /* CONFIG_NO_FORK */

	free(argv);

#endif /* defined(CONFIG_NO_FORK) && defined(CONFIG_NO_SPAWN) */

	return ret;
}


/* Runs a programme to checks MRs. */

int
run_mr_checker(const char *prg, const char *arg1, list<mystring> mrs) {
	list<const char *> args;

	args.add(arg1);
	args += mrs;

	return run(prg, args);
}

/* Local variables: */
/* mode: c++ */
/* End: */
