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

#include "cssc.h"
#include "mystring.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: mystring.cc,v 1.4 1997/05/10 14:49:51 james Exp $";
#endif


void
mystring::create(const char *s) {
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
mystring::create(const char *s1, const char *s2) {
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

mystring::mystring(const char *s, size_t len)
{
  ptr *p = (ptr *) xmalloc(STR_OFFSET + len + 1);
  memcpy(p->str, s, len);
  p->str[len] = '\0';
  p->refs = 1;
  str = (char *) p + STR_OFFSET;
}

const mystring& mystring::operator+=(const mystring& s)
{
  size_t newlen = strlen(str) + strlen(s.str) + 1;
  ptr *p = (ptr *) xmalloc(STR_OFFSET + newlen);
  strcpy(p->str, str);
  strcat(p->str, s.str);
  destroy();
  create(p->str);
  free(p);
  return *this;
}


mystring mystring::operator+(const mystring& s)
{
  mystring ret(str);
  ret += s;
  return ret;
}


void
mystring::destroy() {
	if (str != NULL && --STR_PTR(str)->refs == 0) {
		free(STR_PTR(str));
	}
}

mystring &
mystring::operator =(mystring const &s) {
	if (this != &s) {
		destroy();
		copy(s);
	}
	return *this;
}

mystring &
mystring::operator =(const char *s) {
	assert(s == NULL || str != s);
	destroy();
	create(s);
	return *this;
}
	
/* Local variables: */
/* mode: c++ */
/* End: */

