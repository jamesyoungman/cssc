/*
 * sl-merge.h: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 2007, 2008, 2009, 2010, 2011, 2014, 2019 Free
 *  Software Foundation, Inc.
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
 * Merge and remove member functions of the template range_list.
 */
#include "cssc.h"
#include "sid.h"
#include "sid_list.h"

#ifndef CSSC__SL_MERGE_H__
#define CSSC__SL_MERGE_H__

template <class TYPE>
range_list<TYPE> &
range_list<TYPE>::merge(range_list<TYPE> const &list) {
	if (!valid() || !list.valid()) {
		return *this;
	}

	range<TYPE> *sp = list.head_;
	if (sp == NULL) {
		return *this;
	}

	ASSERT(valid());
	sp = do_copy_list(sp);

	if (head_ == NULL) {
		head_ = sp;
	} else {
		range<TYPE> *dp = head_;

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

	range<TYPE> *sp = list.head_;
	if (sp == NULL) {
		return *this;
	}

	if (head_ == NULL) {
		return *this;
	}

	while(sp != NULL) {
		range<TYPE> *dp = head_;
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
				range<TYPE> *p = new range<TYPE>;
				p->from = dp->from;
				p->to = sp->from;
				--p->to;

				sp->from = dp->to;
				++sp->from;

				p->next = head_->next;
				head_ = p;
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
