/*
 * prompt.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Defines the function prompt_user.
 *
 */

#include "cssc.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: prompt.cc,v 1.3 1997/05/10 14:49:52 james Exp $";
#endif

/* Prompts the user for input. */

mystring
prompt_user(const char *prompt) {
	static char *linebuf = (char *) xmalloc(CONFIG_LINEBUF_CHUNK_SIZE);
	static int buflen = CONFIG_LINEBUF_CHUNK_SIZE;
	int c;
	int i = 0;

	fputs(prompt, stdout);
	fflush(stdout);
	
	c = getchar();
	while(c != EOF && c != '\n') {
		if (i == buflen - 1) {
			buflen += CONFIG_LINEBUF_CHUNK_SIZE;
			linebuf = (char *) xrealloc(linebuf, buflen);
		}
		linebuf[i++] = c;
		c = getchar();
	}

	linebuf[i] = '\0';
	return linebuf;
}

/* Local variables: */
/* mode: c++ */
/* End: */
