/*
 * sid_list.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Member functions of the template range_list.
 *
 * @(#) MySC sid_list.c 1.1 93/11/09 17:18:03
 *
 */

#ifndef __SID_LIST_C__
#define __SID_LIST_C__

#ifdef TEST
#include "mysc.h"
#include "linebuf.h"
#include "sid.h"
#include "quit.c"
#include "xalloc.c"
#include "linebuf.c"
#include "sid.c"
#endif

#ifdef __GNUC__
#pragma not really interface
#endif

template <class TYPE>
range_list<TYPE>::range_list(const char *list): head(NULL) {
	const char *s = list;
	if (s == NULL || *s == '\0') {
		return;
	}

	do {
		const char *comma = strchr(s, ',');

		if (comma == NULL) {
			comma = s + strlen(s);
		}
		
		char buf[64];
		size_t len = comma - s;
		if (len > sizeof(buf) - 1) {
			quit(-1, "Range in list too long: '%s'",
			     list);
		}

		memcpy(buf, s, len);
		buf[len] = '\0';

		char *dash = strchr(buf, '-');
		struct range *p = (struct range *)
				  xmalloc(sizeof(struct range));

		if (dash == NULL) {
			p->to = p->from = TYPE(buf);
		} else {
			*dash++ = '\0';
			p->from = TYPE(buf);
			p->to = TYPE(dash);
		}

		p->next = head;
		head = p;

		s = comma;
	} while(*s++ != '\0');

	if (clean()) {
		destroy();
		head = (struct range *) invalid_range;
	}	
}

template <class TYPE>
int
range_list<TYPE>::clean() {
	if (!valid()) {
		return 1;
	}

	int invalid = 0;
	struct range *sp = head;
	struct range *new_head = NULL;
	while(sp != NULL) {
		struct range *next_sp = sp->next;

		if (sp->from <= sp->to) {
			struct range *dp = new_head;
			struct range *pdp = NULL;
			TYPE sp_to_1 = sp->to;
			TYPE sp_from_1 = sp->from;
			++sp_to_1;
			--sp_from_1;

			while(dp != NULL && dp->to < sp_from_1) {
				pdp = dp;
				dp = dp->next;
			}

			while(dp != NULL && dp->from <= sp_to_1) {
				/* While sp overlaps dp, merge dp into sp. */
				if (dp->to > sp->to) {
					sp_to_1 = sp->to = dp->to;
					++sp_to_1;
				}
				if (dp->from < sp->from){
					sp->from = dp->from;
				}

				struct range *next_dp = dp->next;
				free(dp);
				dp = next_dp;
				if (pdp == NULL) {
					new_head = dp;
				} else {
					pdp->next = dp;
				}
			}
			if (pdp == NULL) {
				sp->next = new_head; 
				new_head = sp;
			} else {
				sp->next = pdp->next;
				pdp->next = sp;
			}
		} else {
			invalid = 1;
			free(sp);
		}
		sp = next_sp;
	}
	head = new_head;
	return invalid;
}		
				
template <class TYPE>
int
range_list<TYPE>::member(TYPE id) const {
	struct range const *p = head;

	while(p != NULL) {
		if (p->from <= id && id <= p->to) {
			return 1;
		}
		p = p->next;
	}

	return 0;
}

template <class TYPE>
void
range_list<TYPE>::destroy() {
	if (!valid()) {
		return;
	}

	struct range *p = head;

	while(p != NULL) {
		struct range *np = p->next;
		free(p);
		p = np;
	}

	head = NULL;
}

template <class TYPE>
void *
range_list<TYPE>::_copy(void *_p) {
	struct range *p = (struct range *) _p;
	if (p == NULL) {
		return NULL;
	}
	if (p == invalid_range) {
		return p;
	}

	struct range *head = (struct range *) xmalloc(sizeof(struct range));
	struct range *np = head;

	while(1) {
		np->from = p->from;
		np->to = p->to;

		p = p->next;
		if (p == NULL) {
			break;
		}

		np = np->next = (struct range *) xmalloc(sizeof(struct range));
	}

	np->next = NULL;
	return head;
}		

template <class TYPE>
range_list<TYPE> &
range_list<TYPE>::operator =(range_list<TYPE> const &list) {
	struct range *p = copy(list.head);
	destroy();
	head = p;
	return *this;
}	

template <class TYPE>
int
range_list<TYPE>::print(FILE *out) const {
	struct range *p = head;

	if (p == NULL || !valid()) {
		return 0;
	}


	while(1) {
		if (p->from.print(out)) {
			return 1;
		}
		if (p->to != p->from
		    && (putc('-', out) == EOF
			|| p->to.print(out))) {
			return 1;
		}

		p = p->next;
		if (p == NULL) {
			return 0;
		}

		if (putc(',', out) == EOF) {
			return 1;
		}
	}
}

#ifdef TEST

extern "C" int isatty(int);

void usage() {}

int
main() {
	class _linebuf linebuf;
	
	while(isatty(0) && fputs("\n> ", stdout), fflush(stdout),
	      !linebuf.read_line(stdin)) {
		linebuf[strlen(linebuf) - 1] = '\0';

		sid_list test(linebuf);

		printf("\"%s\"\n -> \"", (const char *) linebuf);
		test.print(stdout);
		puts("\"");
	}
	return 0;
}
#endif
			
#endif /* __SID_LIST_C__ */
		
/* Local variables: */
/* mode: c++ */
/* End: */
