/*
 * sid_list.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Defines the template range_list.
 *
 * @(#) MySC sid_list.h 1.1 93/11/09 17:17:51
 *
 */

#ifndef __SID_LIST_H__
#define __SID_LIST_H__

#ifdef __GNUC__
// #pragma this does not work with templates interface
#endif

extern void *invalid_range;

template <class TYPE>
class range_list {
	struct range {
		TYPE from;
		TYPE to;
		struct range *next;
	} *head;

	void destroy();
	static void *_copy(void *list);

	static struct range *
	copy(struct range *p) { 
		return (struct range *)_copy(p);
	}

	int clean();

/*	static void *invalid_range; /**/

public:
	range_list(): head(NULL) {}
	range_list(const char *list);

	range_list(range_list const &list) {
		head = copy(list.head);
	}	

	range_list &operator =(range_list const &list);

	void
	operator =(void *p) {
		assert(p == NULL);
		destroy();
		head = 0;
	}

	operator void const *() const { return (void const *) head; }

	int valid() const { return head != invalid_range; }
	int member(TYPE id) const;

	range_list &merge (range_list const &list);
	range_list &remove (range_list const &list);

	int print(FILE *out) const;

	~range_list() { destroy(); }
};

#include "sid_list.c"
#ifdef CONFIG_COMPLETE_TEMPLATES
#include "sl-merge.c"
#endif

#endif /* __SID_LIST_H__ */
	
/* Local variables: */
/* mode: c++ */
/* End: */


