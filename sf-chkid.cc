/*
 * sf-chkid.c
 *
 * By Ross Ridge
 * Public Domain
 * 
 */

#include "cssc.h"
#include "sccsfile.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-chkid.cc,v 1.3 1997/05/10 14:49:56 james Exp $";
#endif


/* Returns true if the string contains a valid SCCS id keyword. */

int
sccs_file::check_id_keywords(const char *s) {
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
