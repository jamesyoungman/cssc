/*
 * sf-cdc.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Members of the class sccs_file used for change the comments and
 * MRs of a delta. 
 *
 */

#include "cssc.h"
#include "sccsfile.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-cdc.cc,v 1.3 1997/05/10 14:49:55 james Exp $";
#endif

/* Adds new MRs and comments to the specified delta. */

void
sccs_file::cdc(sid id, list<mystring> mrs, list<mystring> comments) {
	int i;
	int len;

	struct delta *p = (struct delta *) delta_table.find(id); /* !!! */
	if (p == NULL) {
		quit(-1, "%s: Requested SID doesn't exist.",
		     (const char *) name);
	}
	struct delta &delta = *p;

	list<mystring> not_mrs;
	len = mrs.length();
	for(i = 0; i < len; i++) {
		const char *s = mrs[i];
		if (s[0] == '!') {
			not_mrs.add(s + 1);
		} else {
			delta.mrs.add(s);
		}
	}

	delta.mrs -= not_mrs;

	delta.comments += comments;
}

/* Local variables: */
/* mode: c++ */
/* End: */
