/* 
 * sccsname.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Members of the class sccs_name.
 *
 */

#ifdef __GNUC__
#pragma implementation "sccsname.h"
#endif

#include "mysc.h"
#include "sccsname.h"

#include <ctype.h>

#ifdef CONFIG_SCCS_IDS
static const char sccs_id[] = "@(#) MySC sccsname.c 1.1 93/11/09 17:17:59";
#endif

const char *
base_part(const char *name) {
	const char *s = name + strlen(name);
	
#ifdef CONFIG_MSDOS_FILES
	if (isalpha(name[0]) && name[1] == ':') {
		name += 2;
	}

	while(s > name && *s != '/' && *s != '\\') {
		s--;
	}
#else
	while(s > name && *s == '/') {
		s--;
	}

	while(s > name && *s != '/') {
		s--;
	}
#endif

	if (s == name) {
		return name;
	}

	return s + 1;
}

int
sccs_name::valid_filename(const char *name) {
	const char *base = base_part(name);

#ifdef CONFIG_MSDOS_FILES
	const char *dot = strrchr(base, '.');
	const char *dollar = strrchr(base, '$');

	return dot != NULL && dollar != NULL && dollar > dot
	       && strchr(base, '.') == dot;
#else
	return base[0] == 's' && base[1] == '.';
#endif
}

void
sccs_name::create() {
	if (!valid_filename(name)) {
		_name = NULL;
		return;
	}

	_name = name.xstrdup();

#ifdef CONFIG_MSDOS_FILES

	char *s = (char *) base_part(_name);
	change = s + strlen(s) - 1;

	if (change > s && change[-1] == '.') {
		change[-1] = '\0';
		gname = base_part(_name);
		change[-1] = '.';
	} else {
		gname = base_part(_file('\0'));
	}

#else 

	change = (char *) base_part(_name);
	gname = change + 2;

#endif /* CONFIG_MSDOS_FILES */
}

sccs_name &
sccs_name::operator =(mystring n) {
	destroy();
	name = n;
	create();
	return *this;
}

#ifdef CONFIG_FILE_NAME_GUESSING

void
sccs_name::make_valid() {
	assert(!valid_filename(name));

#ifdef CONFIG_MSDOS_FILES

	char *s = name.xstrdup();
	char *base = (char *) base_part(s);
	char *dot = strchr(base, '.');

	if (dot == NULL) {
		name = mystring(s, ".$");
	} else if (dot[1] == '\0' || dot[2] == '\0' || dot[3] == '\0') {
		name = mystring(s, "$");
	} else {
		dot[3] = '$';
		dot[4] = '\0';
		name = s;
	}

	if (!is_readable(name)) {
		s = name.xstrdup();
		base = (char *) base_part(s);

		mystring tmp(strchr(s, '/') != NULL ? "SCCS/" : "SCCS\\", base);
		base[0] = '\0';
		tmp = mystring(s, tmp);

		if (is_readable(tmp)) {
			name = tmp;
		}
	}


	free(s);

#else /* CONFIG_MSDOS_FILES */

	char *s = name.xstrdup();
	char *base = (char *) base_part(s);

	mystring tmp("s.", base);
	base[0] = '\0';
	name = mystring(s, tmp);
	       
	if (!is_readable(name)) {
		tmp = mystring("SCCS/", tmp);
		tmp = mystring(s, tmp);
		if (is_readable(tmp)) {
			name = tmp;
		}
	}

	free(s);

#endif /* CONFIG_MSDOS_FILES */

	create();
}
	
#endif /* CONFIG_FILE_NAME_GUESSING */

/* Local variables: */
/* mode: c++ */
/* End: */
