/*
 * sid_list.c: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997, Free Software Foundation, Inc. 
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Member functions of the template range_list.
 *
 * @(#) CSSC sid_list.c 1.1 93/11/09 17:18:03
 *
 */

#ifndef __SID_LIST_C__
#define __SID_LIST_C__


#ifdef TEST
#include <stdarg.h>
#include <stdlib.h>
#include "cssc.h"
#include "list.h"
#include "linebuf.h"
#include "sid.h"
#include "xalloc.cc"
#include "linebuf.cc"
#include "sid.cc"

// TEST - only.
NORETURN
quit(int err, const char *fmt, ...)
{
  va_list ap;
  ap = va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  exit(abs(err));
}
NORETURN nomem() { quit(-4, "Out of memory!"); }
NORETURN assert_failed(char const *file, int line, char const *expr)
{
  quit(-1, "assertion \"%s\" failed at %s, line %d\n", expr, file, line);
}


#endif

#ifdef __GNUC__
#pragma not really interface
#endif

template <class TYPE> range_list<TYPE>::range_list(const char *list)
  : head(NULL), valid_flag(1)
{
  const char *s = list;
  if (s == NULL || *s == '\0')
    {
      return;
    }

  do
    {
      const char *comma = strchr(s, ',');

      if (comma == NULL)
	{
	  comma = s + strlen(s);
	}
		
      char buf[64];
      size_t len = comma - s;
      if (len > sizeof(buf) - 1)
	{
	  quit(-1, "Range in list too long: '%s'",
	       list);
	}

      memcpy(buf, s, len);
      buf[len] = '\0';

      char *dash = strchr(buf, '-');
      range<TYPE> *p = (range<TYPE> *) xmalloc(sizeof(range<TYPE>));

      if (dash == NULL)
	{
	  p->to = p->from = TYPE(buf);
	}
      else
	{
	  *dash++ = '\0';
	  p->from = TYPE(buf);
	  p->to = TYPE(dash);
	}

      p->next = head;
      head = p;

      s = comma;
    } while(*s++ != '\0');

  if (clean())			// returns invalid flag.
    {
      destroy();
      head = NULL;
      invalidate();
    }
  else
    {
      assert(valid());
    }
}

template <class TYPE>
int
range_list<TYPE>::clean()
{
  if (!valid())
    return 1;
  
  range<TYPE> *sp = head;
  range<TYPE> *new_head = NULL;
  while (sp != NULL)
    {
      range<TYPE> *next_sp = sp->next;

      if (sp->from <= sp->to)
	{
	  range<TYPE> *dp = new_head;
	  range<TYPE> *pdp = NULL;
	  
	  TYPE sp_to_1 = sp->to;
	  TYPE sp_from_1 = sp->from;
	  ++sp_to_1;
	  --sp_from_1;

	  while (dp != NULL && dp->to < sp_from_1)
	    {
	      pdp = dp;
	      dp = dp->next;
	    }

	  while (dp != NULL && dp->from <= sp_to_1)
	    {
	      /* While sp overlaps dp, merge dp into sp. */
	      if (dp->to > sp->to)
		{
		  sp_to_1 = sp->to = dp->to;
		  ++sp_to_1;
		}
	      if (dp->from < sp->from)
		{
		  sp->from = dp->from;
		}

	      range<TYPE> *next_dp = dp->next;
	      free(dp);
	      dp = next_dp;
	      if (pdp == NULL)
		{
		  new_head = dp;
		}
	      else
		{
		  pdp->next = dp;
		}
	    }
	  if (pdp == NULL)
	    {
	      sp->next = new_head; 
	      new_head = sp;
	    }
	  else
	    {
	      sp->next = pdp->next;
	      pdp->next = sp;
	    }
	}
      else
	{
	  invalidate();
	  free(sp);
	}
      sp = next_sp;
    }
  head = new_head;
  return !valid_flag;
}		
				
template <class TYPE>
int
range_list<TYPE>::member(TYPE id) const
{
  const range<TYPE> *p = head;

  while (p)
    {
      if (p->from <= id && id <= p->to)
	{
	  return 1;		// yes
	}
      p = p->next;
    }
  return 0;			// no
}

template <class TYPE>
void
range_list<TYPE>::destroy()
{
  if (!valid())
      return;

  range<TYPE> *p = head;

  while(p != NULL)
    {
      range<TYPE> *np = p->next;
      free(p);
      p = np;
    }
  head = NULL;
}

template <class TYPE> 
range<TYPE> *
range_list<TYPE>::do_copy_list(range<TYPE> *p) // static member.
{
  if (p == NULL)
    {
      return NULL;
    }
  range<TYPE> *copy_head = (range<TYPE>	*) xmalloc(sizeof(range<TYPE>));
  range<TYPE> *np = copy_head;
  
  while(1)
    {
      np->from = p->from;
      np->to = p->to;
      
      p = p->next;
      if (p == NULL)
	{
	  break;
	}

      np = np->next = (range<TYPE> *) xmalloc(sizeof(range<TYPE>));
    }

  np->next = NULL;
  return copy_head;
}		

template <class TYPE>
range_list<TYPE>::range_list(range_list const &list)
{
  valid_flag = 1;
  assert(list.valid());
  head = do_copy_list(list.head);
  assert(valid());
}	

template <class TYPE>
range_list<TYPE> &
range_list<TYPE>::operator =(range_list<TYPE> const &list)
{
  assert(valid());
  assert(list.valid());

  range<TYPE> *p = do_copy_list(list.head);
  destroy();
  head = p;

  assert(valid());
  return *this;
}	


template <class TYPE>
range_list<TYPE>::range_list(): head(0), valid_flag(1) 
{
}

template <class TYPE>
int
range_list<TYPE>::valid() const
{
  return valid_flag;
}

template <class TYPE>
int
range_list<TYPE>::empty() const
{
  return head ? 0 : 1;
}

template <class TYPE>
void
range_list<TYPE>::invalidate()
{
  valid_flag = 0;
}

template <class TYPE>
range_list<TYPE>::~range_list()
{
  destroy();
  invalidate();
}

template <class TYPE>
int
range_list<TYPE>::print(FILE *out) const
{
  if (empty() || !valid())
    {
      return 0;
    }


  range<TYPE> *p = head;
  while (1)
    {
      if (p->from.print(out))
	{
	  return 1;
	}
      if (p->to != p->from
	  && (putc('-', out) == EOF
	      || p->to.print(out)))
	{
	  return 1;
	}

      p = p->next;
      if (p == NULL)
	{
	  return 0;
	}

      if (putc(',', out) == EOF)
	{
	  return 1;
	}
    }
}

#ifdef TEST

extern "C" int isatty(int);

void usage()
{
}

void prompt()
{
  fputs("\n> ", stdout);
  fflush(stdout);
}

int
main()
{
  class _linebuf linebuf;

  // create & destroy a sid_list first.
  if (1)
    {
      sid_list x;
    }
  
  // Other constructors...
  if (1)
    {
      sid_list a;
      sid_list b(a);
      sid_list c = a;
    }
  
  if (!isatty(0))
    {
      printf("No interactive test -- stdin is not a tty.\n");
      return 0;
    }

  for (prompt(); !linebuf.read_line(stdin); prompt())
    {
      linebuf[strlen(linebuf) - 1] = '\0';
      printf("\"%s\"\n -> ", (const char *) linebuf);
      
      sid_list test(linebuf);
      
      fputs("\"", stdout);
      test.print(stdout);
      fputs("\"", stdout);
    }
  return 0;
}

// Explicit template instantiations -- only when TEST is defined.
//template class range_list<sid>;
//template class list<sid>;
//template class list<mystring>;

//#include "stack.h"
//template class stack<unsigned short>;

//#include "sid_list.h"
//template class range_list<release>;

#endif
			
#endif /* __SID_LIST_C__ */
		
/* Local variables: */
/* mode: c++ */
/* End: */
