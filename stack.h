/*
 * stack.h: Part of GNU CSSC.
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
 * Defines a simple template that implements a stack for types without
 * constructors and destructors. 
 *
 * @(#) CSSC stack.h 1.2 93/11/13 00:12:55
 *
 */

#ifndef __STACK_H__
#define __STACK_H__

template <class TYPE>
class stack {
	TYPE *array;
	int top;
	int len;

	void
	copy(class stack<TYPE> const &it) {
		len = it.len;
		array = (TYPE *) xmalloc(sizeof(TYPE) * len);
		memcpy(array, it.array, sizeof(TYPE) * len);
		top = it.top;
	}
	       
public:
	stack(int l):
	  array((TYPE *)xmalloc(l * sizeof(TYPE))),
	  top(0), len(l) {}

	stack(stack<TYPE> const &it) {
		copy(it);
	}

	class stack<TYPE> &
        operator =(class stack<TYPE> const &it) {
		if (this != &it) {
			free(array);
			copy(it);
		}
		return *this;
	}

	void
	push(TYPE a) {
		assert(top < len);
		array[top++] = a;
	}

	TYPE
	pop() {
		assert(top > 0);
		return array[--top];
	}

	int empty() const { return top == 0; }

	~stack() { free(array); }
};

#endif /* __STACK_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
