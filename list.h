/*
 * list.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Defines the template list.
 *
 * @(#) MySC list.h 1.1 93/11/09 17:17:47
 *
 */
 
#ifndef __LIST_H__
#define __LIST_H__

template <class TYPE>
class list {
protected:
	TYPE *array;
	int len;
	int left;

	void destroy();
	void copy(list const &l);

public:
	list(): len(0), left(0) {}
	list(list const &l) { copy(l); }
	
	list &operator =(list const &l);

	void
	operator =(void *p) {
		assert(p == NULL);
		destroy();
		len = 0;
		left = 0;
	}

	void add(TYPE const &ent);

	int length() const { return len; }

	TYPE const &
	operator [](int index) const {
		assert(index >= 0 && index < len);
		return array[index];
	}

	TYPE &
	select(int index) const {
		assert(index >= 0 && index < len);
		return array[index];
	}

	~list() { destroy(); }
};

#include "list.cc"


/* Appends the contents of one list to another. */

template<class T1, class T2> 
list<T1> &
operator +=(list<T1> &l1, list<T2> const &l2) {
	int i;
	int len = l2.length();
	for(i = 0; i < len; i++) {
#ifdef __GNUC__
		l1.add((T1) l2[i]);
#else
		l1.add(l2[i]);
#endif
	}
	return l1;
}


/* Deletes the contents of one list from another. */

template<class T1, class T2> 
list<T1> &
operator -=(list<T1> &l1, list<T2> const &l2) {
	int i, j;
	int len1 = l1.length();
	int len2 = l2.length();
	
	if (len2 > 0) {
		list<T1> nl;

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

#endif /* __LIST_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
