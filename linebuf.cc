/*
 * linebuf.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Members of the class _linebuf.
 *
 */

#ifdef __GNUC__
#pragma implementation "linebuf.h"
#endif

#include "cssc.h"
#include "linebuf.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: linebuf.cc,v 1.3 1997/05/10 14:49:50 james Exp $";
#endif

int
_linebuf::read_line(FILE *f) {
	buf[buflen - 2] = '\0';

	char *s = fgets(buf, buflen, f);
	while(s != NULL) {
		char c = buf[buflen - 2];
		if (c == '\0' || c == '\n') {
#if 0
			printf("%s", buf);
#endif
			return 0;
		}

		buf = (char *) xrealloc(buf,
					CONFIG_LINEBUF_CHUNK_SIZE + buflen);
		s = buf + buflen - 1;
		buflen += CONFIG_LINEBUF_CHUNK_SIZE;
		buf[buflen - 2] = '\0';
		
#if 1
		fprintf(stderr, "buflen = %d\n", buflen);

#endif		
		s = fgets(s, CONFIG_LINEBUF_CHUNK_SIZE + 1, f);
	}

	return 1;
}

/* Local variables: */
/* mode: c++ */
/* End: */
