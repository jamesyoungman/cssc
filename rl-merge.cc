/*
 * rl-merge.cc: Part of GNU CSSC.
 *
 * Copyright (C) 1997  Free Software Foundation, Inc.
 *                     675 Mass Ave, Cambridge, MA 02139, USA
 *
 */

#include "cssc.h"
#include "rel_list.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: rl-merge.cc,v 1.1 1997/05/31 10:18:36 james Exp $";
#endif

// another horrendously inefficient implementation.
void release_list::merge(const release_list& m)
{
  const int mlen = m.l.length();
  for(int i=0; i<mlen; i++)
    {
      const release r = m.l[i];
      if (!member(r))
	l.add(r);
    }
}

// another horrendously inefficient implementation.
void release_list::remove(const release_list& rm)
{
  const int len = l.length();
  list<release> newlist;
  
  for(int i=0; i<len; i++)
    {
      const release r = l[i];
      if (!rm.member(r))
	newlist.add(r);
    }
  l = newlist;
}



/* Local variables: */
/* mode: c++ */
/* End: */
