/*
 * list.c: Part of GNU CSSC.
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
 * Test code for the template list<T>.
 */

#ifndef CSSC__LIST_C__
#define CSSC__LIST_C__

template <class TYPE>
void
list<TYPE>::destroy() {
	if (len != 0) {
		delete[] array;
	}
}

template <class TYPE>
void
list<TYPE>::copy(list<TYPE> const &l) {
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
list<TYPE> &
list<TYPE>::operator =(list<TYPE> const &l) {
	if (this != &l) {
		destroy();
		copy(l);
	}
	return *this;
}

template <class TYPE>
void
list<TYPE>::add(TYPE const &ent) {
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


#endif

/* Local variables: */
/* mode: c++ */
/* End: */
