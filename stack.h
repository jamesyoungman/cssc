/*
 * stack.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Defines a simple template that implements a stack for types without
 * constructors and destructors. 
 *
 * @(#) MySC stack.h 1.2 93/11/13 00:12:55
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
