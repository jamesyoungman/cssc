/*
 * pf-add.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Members of the class sccs_pfile for adding an edit lock to the file.
 *
 */

#include "mysc.h"
#include "pfile.h"

#ifdef CONFIG_SCCS_IDS
static char const sccs_id[] = "@(#) MySC pf-add.c 1.1 93/11/09 17:17:56";
#endif

void
sccs_pfile::add_lock(sid got, sid delta, 
		     sid_list &included, sid_list &excluded) {
	assert(mode == APPEND);

	struct edit_lock new_lock;

	new_lock.got = got;
	new_lock.delta = delta;
	new_lock.user = get_user_name();
	new_lock.date = sccs_date::now();
	new_lock.include = included;
	new_lock.exclude = excluded;
	new_lock.deleted = 0;

	edit_locks.add(new_lock);

	FILE *pf;
	if (edit_locks.length() == 0) {
		pf = fcreate(pname, CREATE_EXCLUSIVE);
		if (pf == NULL) {
			quit(errno, "%s: Can't create.", (char const *) pname);
		}
	} else {
		pf = fopen(pname, "a");
		if (pf == NULL) {
			quit(errno, "%s: Can't open for append.",
			     (char const *) pname);
		}
	}

	if (write_edit_lock(pf, new_lock)
	    || fclose(pf) == EOF) {
		quit(errno, "%s: Write error.", (char const *) pname);
	}
}

/* Local variables: */
/* mode: c++ */
/* End: */
