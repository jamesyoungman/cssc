/*
 * mystring.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Members of the class string.
 *
 */

#ifdef __GNUC__
#pragma implementation "mystring.h"
#endif

#include "mysc.h"

#ifdef CONFIG_SCCS_IDS
static char const sccs_id[] = "@(#) MySC mystring.c 1.1 93/11/09 17:32:32";
#endif


void
string::create(char const *s) {
	if (s == NULL) {
		str = NULL;
		return;
	}

	ptr *p = (ptr *) xmalloc(STR_OFFSET + strlen(s) + 1);
	strcpy(p->str, s);
	p->refs = 1;

	str = (char *) p + STR_OFFSET;
}

void
string::create(char const *s1, char const *s2) {
	if (s1 == NULL) {
		create(s2);
		return;
	}
	if (s2 == NULL) {
		create(s1);
		return;
	}

	int len1 = strlen(s1);
	ptr *p = (ptr *) xmalloc(STR_OFFSET + len1 + strlen(s2) + 1);
	strcpy(p->str, s1);
	strcpy(p->str + len1, s2);
	p->refs = 1;

	str = (char *) p + STR_OFFSET;
}


void
string::destroy() {
	if (str != NULL && --STR_PTR(str)->refs == 0) {
		free(STR_PTR(str));
	}
}

string &
string::operator =(string const &s) {
	if (this != &s) {
		destroy();
		copy(s);
	}
	return *this;
}

string &
string::operator =(char const *s) {
	assert(s == NULL || str != s);
	destroy();
	create(s);
	return *this;
}
	
/* Local variables: */
/* mode: c++ */
/* End: */

