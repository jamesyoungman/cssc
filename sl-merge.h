/*
 * sl-merge.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Merge and remove member functions of the template range_list.
 *
 * @(#) CSSC sl-merge.c 1.1 93/11/09 17:18:03
 *
 */
#include "cssc.h"
#include "sid.h"
#include "sid_list.h"

#ifndef __SL_MERGE_C__
#define __SL_MERGE_C__

template <class TYPE>
range_list<TYPE> &
range_list<TYPE>::merge(range_list<TYPE> const &list) {
	if (!valid() || !list.valid()) {
		return *this;
	}

	struct range *sp = list.head;
	if (sp == NULL) {
		return *this;
	}

	sp = copy(sp);

	if (head == NULL) {
		head = sp;
	} else {
		struct range *dp = head;

		while(dp->next != NULL) {
			dp = dp->next;
		}

		dp->next = sp;
	}
	clean();
	return *this;
}

template <class TYPE>
range_list<TYPE> &
range_list<TYPE>::remove(range_list<TYPE> const &list) {
	if (!valid() || !list.valid()) {
		return *this;
	}

	struct range *sp = list.head;
	if (sp == NULL) {
		return *this;
	}

	if (head == NULL) {
		return *this;
	}

	while(sp != NULL) {
		struct range *dp = head;
		while(dp != NULL) {
			if (sp->from <= dp->from
			    && sp->to >= dp->from) {
				dp->from = sp->to;
				++dp->from;
			} 
			if (sp->to >= dp->to
			    && sp->from <= dp->to) {
				dp->to = sp->from;
				--dp->to;
			}
			if (sp->from > dp->from && sp->to < dp->to) {
				struct range *p = (struct range *) 
					xmalloc(sizeof(struct range));
				p->from = dp->from;
				p->to = sp->from;
				--p->to;

				sp->from = dp->to;
				++sp->from;

				p->next = head->next;
				head = p;
			}
			dp = dp->next;
		}
		sp = sp->next;
	}
	clean();
	return *this;
}

#endif /* __SL_MERGE_C__ */

/* Local variables: */
/* mode: c++ */
/* End: */
