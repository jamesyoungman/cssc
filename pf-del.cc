/*
 * pf-del.c
 *
 * By Ross Ridge 
 * Public Domain
 *
 * Members of the class sccs_pfile used when removing an edit lock from
 * a p-file.
 *
 */

#include "mysc.h"
#include "pfile.h"

#ifdef CONFIG_SCCS_IDS
static const char sccs_id[] = "@(#) MySC pf-del.c 1.2 93/11/13 00:12:06";
#endif

/* enum */ sccs_pfile::find_status
sccs_pfile::find_sid(sid id) {

	rewind();

	const char *user = get_user_name();
	sccs_pfile &it = *this;
	int found = -1;

	while(next()) {
		if (strcmp(user, it->user) == 0
		    && (id.is_null() || id == it->got || id == it->delta)) {
			if (found != -1) {
				return AMBIGUOUS;
			}
			found = pos;
		}
	}

	if (found == -1) {
		return NOT_FOUND;
	}

	pos = found;

	return FOUND;
}

void
sccs_pfile::update() {
	mystring qname = name.qfile();

	FILE *pf = fopen(qname, "w");
	if (pf == NULL) {
		quit(errno, "%s: Can't create temporary file.",
		     (const char *) qname);
	}

        int count = 0;

	rewind();
	while(next()) {
#ifdef __GNUC__
		if (write_edit_lock(pf, edit_locks[pos])) {
			quit(errno, "%s: Write error.", (const char *) qname);
		}
		    
#else
		if (write_edit_lock(pf, *operator->())) {
			quit(errno, "%s: Write error.", (const char *) qname);
		}
#endif	       
		count++;
	}

	if (fclose(pf) == EOF) {
		quit(errno, "%s: Write error.", (const char *) qname);
	}

#ifndef TESTING	

	if (remove(pname) != 0) {
		quit(errno, "%s: Can't remove old p-file.",
		     (const char *) pname);
	}

	if (count == 0) {
		if (remove(qname) != 0) {
			quit(errno, "%s: Can't remove temporary file.",
			     (const char *) pname);
		}
	} else {
		if (rename(qname, pname) != 0) {
			quit(errno, "%s: Can't rename new p-file.",
			     (const char *) qname);
		}
	}
#endif
}

/* Local variables: */
/* mode: c++ */
/* End: */
