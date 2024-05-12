/*
 * sid_list.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 2001, 2007, 2008, 2009, 2010, 2011, 2014, 2019,
 *  2024 Free Software Foundation, Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * CSSC was originally Based on MySC, by Ross Ridge, which was
 * placed in the Public Domain.
 *
 *
 * Defines the template range_list.
 */

#ifndef CSSC__SID_LIST_H__
#define CSSC__SID_LIST_H__

#include <cstdio>
#include <cstring>

#include "quit.h"


template <class TYPE>
struct range
{
  TYPE from;
  TYPE to;
  range<TYPE> *next;

  range()
    : from(), to(), next(nullptr)
  {
  }
};

template <class TYPE>
class range_list
{
public:
  // constructors & destructors.
  ~range_list();
  range_list();
  range_list(const char *list);
  range_list(range_list const &list);
  range_list &operator =(range_list const &list);

  // query functions.
  bool valid() const;
  bool empty() const;
  bool member(TYPE id) const;

  // manipulation.
  void invalidate();
  range_list &merge  (range_list const &list);
  range_list &remove (range_list const &list);

  // output.
  int print(FILE *out) const;

private:
  // Data members.
  // TODO: rename member variables to consistently have a trailing "_".
  range<TYPE> *head_;
  bool valid_flag_;

  // Implementation.
  void destroy();
  static range<TYPE> *do_copy_list(range<TYPE> *);

  int clean(); // clean the range list eliminating overlaps etc.
};


template <class TYPE> range_list<TYPE>::range_list(const char *list)
    : head_(nullptr),
      valid_flag_(1)
{
  const char *s = list;
  if (s == nullptr || *s == '\0')
    {
      return;
    }

  do
    {
      size_t len;
      const char *comma = strchr(s, ',');
      if (comma == nullptr)
        {
	  len = strlen(s);
          comma = s + len;
        }
      else
	{
	  len = static_cast<size_t>(comma - s);
	}

      char buf[64];
      if ((len+1u) > sizeof(buf))
        {
          ctor_fail(-1, "Range in list too long: '%s'", list);
        }
      else if (len)
        {
          /* SourceForge bug number #438857:
           * ranges like "1.1.1.2," cause an assertion
           * failure while SCCS just ignores the empty list item.
           * Hence we introduce the conditional surrounding this
           * block.
           */
          memcpy(buf, s, len);
          buf[len] = '\0';

          char *dash = strchr(buf, '-');
          range<TYPE> *p = new range<TYPE>;

          if (dash == nullptr)
            {
              p->to = p->from = TYPE(buf);
            }
          else
            {
              *dash++ = '\0';
              p->from = TYPE(buf);
              p->to = TYPE(dash);
            }

          p->next = head_;
          head_ = p;
        }
      s = comma;
    } while(*s++ != '\0');

  if (clean())                  // returns invalid flag.
    {
      destroy();
      head_ = nullptr;
      invalidate();
    }
  else
    {
      ASSERT(valid());
    }
}

template <class TYPE>
int
range_list<TYPE>::clean()
{
  if (!valid())
    return 1;

  range<TYPE> *sp = head_;
  range<TYPE> *new_head = nullptr;
  while (sp != nullptr)
    {
      range<TYPE> *next_sp = sp->next;

      if (sp->from <= sp->to)
        {
          range<TYPE> *dp = new_head;
          range<TYPE> *pdp = nullptr;

          TYPE sp_to_1 = sp->to;
          TYPE sp_from_1 = sp->from;
          ++sp_to_1;
          --sp_from_1;

          while (dp != nullptr && dp->to < sp_from_1)
            {
              pdp = dp;
              dp = dp->next;
            }

          while (dp != nullptr && dp->from <= sp_to_1)
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
              delete dp;
              dp = next_dp;
              if (pdp == nullptr)
                {
                  new_head = dp;
                }
              else
                {
                  pdp->next = dp;
                }
            }
          if (pdp == nullptr)
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
          delete sp;
        }
      sp = next_sp;
    }
  head_ = new_head;
  return !valid_flag_;
}

template <class TYPE>
bool
range_list<TYPE>::member(TYPE id) const
{
  const range<TYPE> *p = head_;

  while (p)
    {
      if (p->from <= id && id <= p->to)
        {
          return true;
        }
      p = p->next;
    }
  return false;
}

template <class TYPE>
void
range_list<TYPE>::destroy()
{
  if (!valid())
      return;

  range<TYPE> *p = head_;

  while(p != nullptr)
    {
      range<TYPE> *np = p->next;
      delete p;
      p = np;
    }
  head_ = nullptr;
}

template <class TYPE>
range<TYPE> *
range_list<TYPE>::do_copy_list(range<TYPE> *p) // static member.
{
  if (p == nullptr)
    {
      return nullptr;
    }
  range<TYPE> *copy_head = new range<TYPE>;
  range<TYPE> *np = copy_head;

  while(1)
    {
      np->from = p->from;
      np->to = p->to;

      p = p->next;
      if (p == nullptr)
        {
          break;
        }

      np = np->next = new range<TYPE>;
    }

  np->next = nullptr;
  return copy_head;
}

template <class TYPE>
range_list<TYPE>::range_list(range_list const &list)
  : head_(nullptr),
    valid_flag_(1)
{
  ASSERT(list.valid());
  head_ = do_copy_list(list.head_);
  ASSERT(valid());
}

template <class TYPE>
range_list<TYPE> &
range_list<TYPE>::operator =(range_list<TYPE> const &list)
{
  ASSERT(valid());
  ASSERT(list.valid());

  range<TYPE> *p = do_copy_list(list.head_);
  destroy();
  head_ = p;

  ASSERT(valid());
  return *this;
}


template <class TYPE>
range_list<TYPE>::range_list()
  : head_(nullptr),
    valid_flag_(1)
{
}

template <class TYPE>
bool
range_list<TYPE>::valid() const
{
  return valid_flag_;
}

template <class TYPE>
bool
range_list<TYPE>::empty() const
{
  return head_ ? false : true;
}

template <class TYPE>
void
range_list<TYPE>::invalidate()
{
  valid_flag_ = 0;
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


  range<TYPE> *p = head_;
  while (1)
    {
      if (!p->from.print(out).ok())
        {
          return 1;
        }
      if (p->to != p->from
          && (putc('-', out) == EOF
              || !p->to.print(out).ok()))
        {
          return 1;
        }

      p = p->next;
      if (p == nullptr)
        {
          return 0;
        }

      if (putc(',', out) == EOF)
        {
          return 1;
        }
    }
}

#endif /* __SID_LIST_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
