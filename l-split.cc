/*
 * l-split.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Functions for non-destructively spliting a string into a list of
 * strings.
 * 
 */

#include "mysc.h"
#include "list.h"

#ifdef CONFIG_SCCS_IDS
static char const sccs_id[] = "@(#) MySC l-split.c 1.1 93/11/09 17:17:55";
#endif

list<string>
split_mrs(string mrs) {
	list<string> mr_list;

	if (mrs != NULL) {
		char *s = mrs.xstrdup();

		s = strtok(s, " \t\n");
		while(s != NULL) {
			mr_list.add(s);
			s = strtok(s, NULL);
		}
		free(s);
	}

	return mr_list;
}

list<string>
split_comments(string comments) {
	list<string> comment_list;

	if (comments != NULL) {
		char *s = comments.xstrdup();
		char *start;
		char *end;

		start = s;
		end = strchr(s, '\n');
		while(end != NULL) {
			*end++ = '\0';
			comment_list.add(start);
			start = end;
			end = strchr(start, '\n');
		}

		if (*start != '\0') {
			comment_list.add(start);
		}

		free(s);
	}

	return comment_list;
}

/* Local variables: */
/* mode: c++ */
/* End: */
