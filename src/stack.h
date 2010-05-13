/*
 * stack.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,2007 Free Software Foundation, Inc. 
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
 * Defines a simple template that implements a stack for types without
 * constructors and destructors. 
 */

#ifndef CSSC__STACK_H__
#define CSSC__STACK_H__

#include "cssc-assert.h"

template <class TYPE>
class stack {
	TYPE *array;
	int top;
	int len;

	void
	copy(stack<TYPE> const &it) {
		len = it.len;
		array = new TYPE[len];
		memcpy(array, it.array, sizeof(TYPE) * len);
		top = it.top;
	}
	       
public:
	stack(int l):
	  array(new TYPE[l]),
	  top(0), len(l) {}

	stack(stack<TYPE> const &it) {
		copy(it);
	}

	stack<TYPE> &
        operator =(stack<TYPE> const &it) {
		if (this != &it) {
		  delete [] array;
		  copy(it);
		}
		return *this;
	}

	void
	push(TYPE a) {
		ASSERT(top < len);
		array[top++] = a;
	}

	TYPE
	pop() {
		ASSERT(top > 0);
		return array[--top];
	}

	int empty() const { return top == 0; }

	~stack() { delete [] array; }
};

#endif /* __STACK_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
