/*
 * rel_list.cc: Part of GNU CSSC.
 *
 * Copyright (C) 1997  Free Software Foundation, Inc.
 *                     675 Mass Ave, Cambridge, MA 02139, USA
 *
 */

#include "cssc.h"
#include "rel_list.h"
#include <stdlib.h>

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: rel_list.cc,v 1.1 1997/05/31 10:18:22 james Exp $";
#endif

// Because we use member() all the time, this
// is a quadratic algorithm...but N is usually very small.
release_list::release_list(const char *s)
{
  assert(NULL != s);

  char *p;
  while (*s)
    {
      long int n = strtol(s, &p, 10);
      if (p == s)
	break;			// not numeric.

      if (n < 0)
	quit(-1, "ranges not allowed in release lists");
      
      // add the entry if not already a member.
      const release r(n);
      if (!member(r))
	l.add(r);
      
      s = p;
      if (',' == *s)
	s++;
    }
}

// linear search for possible member.
bool release_list::member(release r) const
{
  const int len = l.length();
  for(int i=0; i<len; i++)
    {
      if (l[i] == r)
	return true;
    }
  return false;
}


release_list::release_list()
{
}

release_list::release_list(const release_list &r)
{
  l = r.l;
}

release_list::~release_list()
{
}

bool release_list::print(FILE * out) const
{
  const int len = l.length();
  for(int i=0; i<len; i++)
    {
      if (i > 0)
	fputc(' ', out);
      
      l[i].print(out);
    }
  return true;
}
// Explicit template instantiations...
template class list<release>;

/* Local variables: */
/* mode: c++ */
/* End: */
