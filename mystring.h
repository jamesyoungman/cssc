/*
 * mystring.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Defines the class string.
 *
 * @(#) MySC mystring.h 1.1 93/11/09 17:24:42
 *
 */

#ifndef __MYSTRING_H__
#define __MYSTRING_H__

#ifdef __GNUC__
#pragma interface
#endif

#define STR_OFFSET (offsetof(ptr, str))
#define STR_PTR(str) ((ptr *)((str) - STR_OFFSET))

/* This is a very simple string class.  It's purpose is mainly to
   simplify the allocation of strings. */

class string {
	struct ptr {
		int refs;
		char str[1];
	};

	char *str;

	void create(char const *s);
	void create(char const *s1, char const *s2);
	void destroy();

	void
	copy(string const &s) {
		str = s.str;
		if (str != NULL) {
			STR_PTR(str)->refs++;
		}
	}

public:
	string(): str(NULL) {}
	string(char const *s) { create(s); }
	string(char const *s1, char const *s2) { create(s1, s2); }

	string(string const &s) {
		copy(s);
	}

	string &operator =(string const &s);
	string &operator =(char const *s);

	operator char const *() const {
		return str;
	}

#ifdef __GNUC__
	operator void *() const {
		return str;
	}
#endif

	int
	operator ==(string const &s) const {
		assert(s.str != NULL);
		assert(str != NULL);
		return strcmp(str, s.str) == 0;
	}

	int
	operator ==(char const *s) const {
		if (s == NULL) {
			return str == NULL;
		}
		assert(str != NULL);
		return strcmp(str, s) == 0;
	}
		
	int
	print(FILE *out) const {
		return fputs(str, out) == EOF;
	}
		
	char *
	xstrdup() const {
		return strcpy((char *) xmalloc(strlen(str) + 1), str);
	}

	~string() {
		destroy();
	}
};

#endif /* __MYSTRING_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
