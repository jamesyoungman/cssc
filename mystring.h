/*
 * mystring.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Defines the class mystring.
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

/* This is a very simple string class.  Its purpose is mainly to
   simplify the allocation of strings. */

class mystring {
	struct ptr {
		int refs;
		char str[1];
	};

	char *str;

	void create(const char *s);
	void create(const char *s1, const char *s2);
	void destroy();

	void
	copy(mystring const &s) {
		str = s.str;
		if (str != NULL) {
			STR_PTR(str)->refs++;
		}
	}

public:
	mystring(): str(NULL) {}
	mystring(const char *s) { create(s); }
	mystring(const char *s1, const char *s2) { create(s1, s2); }
        mystring(const char *s, size_t len);

	mystring(mystring const &s) {
		copy(s);
	}

	mystring &operator =(mystring const &s);
	mystring &operator =(const char *s);

  	const mystring& operator+=(const mystring& s);
  	mystring  operator+ (const mystring& s);
  
	operator const char *() const {
		return str;
	}
  	operator bool() const   {
	  return str ? true : false;
  	}
  
#ifdef __GNUC__
	operator void *() const {
		return str;
	}
#endif

	int
	operator ==(mystring const &s) const {
		assert(s.str != NULL);
		assert(str != NULL);
		return strcmp(str, s.str) == 0;
	}
	int operator !=(mystring const &s) const { return !operator==(s); }	

	int
	operator ==(const char *s) const {
		if (s == NULL) {
			return str == NULL;
		}
		assert(str != NULL);
		return strcmp(str, s) == 0;
	}
	int operator !=(const char *s) const { return !operator==(s); }
  
	int
	print(FILE *out) const {
		return fputs(str, out) == EOF;
	}
		
	char *
	xstrdup() const {
		return strcpy((char *) xmalloc(strlen(str) + 1), str);
	}

	~mystring() {
		destroy();
	}
};

#endif /* __MYSTRING_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */