/* 
 * sf-prt.cc: Part of GNU CSSC.
 *
 * Copyright (C) 1997  Free Software Foundation, Inc.
 *                     675 Mass Ave, Cambridge, MA 02139, USA
 *
 * Members of the class sccs_file for doing sccs-prt.
 *
 */


#ifndef INC_REL_LIST_H
#define INC_REL_LIST_H

#include "list.h" // NOT STL LIST !

#include "release.h"


class release_list
{
  list<release> l;

public:
  // Constructors / destructors
  release_list();
  release_list(const release_list& create_from);
  release_list(const char *str);
  ~release_list();

  // Adding and removing members specified in other lists.
  void merge(const release_list& m);
  void remove(const release_list& r);
  
  // I/O
  bool print(FILE *) const;

  // Accessors
  bool empty() const { return 0 == l.length(); }
  bool valid() const { return !empty(); }
  bool member(release r) const;
};







/* Local variables: */
/* mode: c++ */
/* End: */
#endif


