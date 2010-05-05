/*
 * mylist.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998,2007 Free Software Foundation, Inc. 
 * 
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *    
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *    
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Defines the template mylist.
 *
 */
 
#ifndef CSSC__LIST_H__
#define CSSC__LIST_H__

#include <cstdlib>
#include <vector>
#include <set>

#include "cssc-assert.h"

#if 1
template <class TYPE>
class mylist
{
protected:
  typedef typename std::vector<TYPE> impl_type;
  impl_type items_;
  
public:
  mylist()
  {
  }
  
  mylist(mylist const &l)
    : items_(l.items_)
  {
  }
  
  mylist &operator =(mylist const &l)
  {
    items_ = l.items_;
    return *this;
  }
  
  bool operator ==(mylist const &l) const
  {
    return items_ == l.items_;
  }
  
  void
  operator =(void *p)
    {
      ASSERT(p == NULL);
      items_.clear();
    }

  void add(TYPE const &ent)
  {
    items_.push_back(ent);
  }

  size_t length() const
    {
      return items_.size();
    }

  TYPE const &
  operator [](int index) const
    {
      ASSERT(index >= 0 && index < items_.size());
      return items_[index];
    }

  TYPE &
  select(int index)
    {
      ASSERT(index >= 0 && index < items_.size());
      return items_[index];
    }

  const mylist<TYPE>& operator+=(const mylist& other)
  {
    typename impl_type::const_iterator ci;
    for (ci=other.items_.begin(); ci!=other.items_.end(); ++ci)
      {
	items_.push_back(*ci);
      }
  }
  
  const mylist<TYPE>& operator-=(const mylist& other)
  {
    if (other.items_.size())
      {
	impl_type remaining;
	std::set<TYPE> unwanted(other.items_.begin(), other.items_.end());
	
	typename impl_type::const_iterator ci;
	for (ci=items_.begin(); ci!=items_.end(); ++ci)
	  {
	    if (unwanted.find(*ci) == unwanted.end())
	      {
		remaining.push_back(*ci);
	      }
	  }
	items_.swap(remaining);
      }
  }
  

  ~mylist()
  {
  }
};
#else

template <class TYPE>
class old_mylist
{
  enum { CONFIG_LIST_CHUNK_SIZE = 16 };
  
protected:
  TYPE *array;
  int len;
  int left;
  
  void destroy();
  void copy(mylist const &l);
  
public:
  mylist(): len(0), left(0) {}
  mylist(mylist const &l) { copy(l); }
  
  mylist &operator =(mylist const &l);
  
  void
  operator =(void *p)
    {
      ASSERT(p == NULL);
      destroy();
      len = 0;
      left = 0;
    }

  void add(TYPE const &ent);

  int length() const
    {
      return len;
    }
  

  TYPE const &
  operator [](int index) const
    {
      ASSERT(index >= 0 && index < len);
      return array[index];
    }

  TYPE &
  select(int index) const
    {
      ASSERT(index >= 0 && index < len);
      return array[index];
    }

  ~mylist();
};

/* Appends the contents of one mylist to another. */

template<class T1> 
mylist<T1> &
operator +=(mylist<T1> &l1, mylist<T1> const &l2)
{
  int i;
  int len = l2.length();
  for(i = 0; i < len; i++)
    l1.add(l2[i]);
  return l1;
}


#ifndef CSSC__LIST_C__
#define CSSC__LIST_C__

template <class TYPE>
mylist<TYPE>::~mylist()
{
  destroy();
}

template <class TYPE>
void
mylist<TYPE>::destroy() {
	if (len != 0) {
		delete[] array;
	}
}

template <class TYPE>
void
mylist<TYPE>::copy(mylist<TYPE> const &l) {
	len = l.len;
	left = l.left;
	if (len + left > 0) {
		array = new TYPE[len + left];
		int i;
		for(i = 0; i < len; i++) {
			array[i] = l.array[i];
		}
	}
}

template <class TYPE>
mylist<TYPE> &
mylist<TYPE>::operator =(mylist<TYPE> const &l) {
	if (this != &l) {
		destroy();
		copy(l);
	}
	return *this;
}

template <class TYPE>
void
mylist<TYPE>::add(TYPE const &ent) {
	if (left == 0) {
		TYPE *new_array = new TYPE[len + CONFIG_LIST_CHUNK_SIZE];
		if (len != 0) {
			int i;
			for(i = 0; i < len; i++) {
				new_array[i] = array[i];
			}
			delete[] array;
		}
		array = new_array;
		left = CONFIG_LIST_CHUNK_SIZE;
	}
	array[len++] = ent;
	left--;
}


#endif /* __LIST_C__ */


/* Deletes the contents of one my mylist from another. */

template<class T1, class T2> 
mylist<T1> &
operator -=(mylist<T1> &l1, mylist<T2> const &l2) {
	int i, j;
	int len1 = l1.length();
	int len2 = l2.length();
	
	if (len2 > 0) {
		mylist<T1> nl;

		for(i = 0; i < len1; i++) {
			T1 elt = l1[i];
			for(j = 0; j < len2; j++) {
				if (elt == (T1) l2[j]) {
					break;
				}
			}
			if (j == len2) {
				nl.add(elt);
			}
		}

		l1 = nl;
	}

	return l1;
}


template <class T1, class T2>
bool 
operator==(const mylist<T1>& left, const mylist<T2>& right)
{
  const int len = left.length();

  if (len != right.length())
    return false;

  for (int i=0; i<len; i++)
    {
      if (!(left[i] == right[i]))
	return false;
    }
  return true;
}
#endif

#endif /* __LIST_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
