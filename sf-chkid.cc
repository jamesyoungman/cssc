/*
 * sf-chkid.c
 *
 * By Ross Ridge
 * Public Domain
 * 
 */

#include "mysc.h"
#include "sccsfile.h"

#ifdef CONFIG_SCCS_IDS
static char const sccs_id[] = "@(#) MySC sf-chkid.c 1.1 93/11/09 17:17:55";
#endif


/* Returns true if the string contains a valid SCCS id keyword. */

int
sccs_file::check_id_keywords(char const *s) {
	s = strchr(s, '%');
	while(s != NULL) {
		if (s[1] != '\0' 
		    && strchr("MIRLBSDHTEGUYFPQCZWA", s[1]) != NULL 
		    && s[2] == '%') {
			return 1;
		}
		s = strchr(s + 1, '%');
	}
	return 0;
}

/* Local variables: */
/* mode: c++ */
/* End: */
