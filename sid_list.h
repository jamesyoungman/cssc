/*
 * sid_list.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Defines the template range_list.
 *
 * @(#) CSSC sid_list.h 1.1 93/11/09 17:17:51
 *
 */

#ifndef __SID_LIST_H__
#define __SID_LIST_H__

#ifdef __GNUC__
// #pragma this does not work with templates interface
#endif

template <class TYPE>
struct range
{
  TYPE from;
  TYPE to;
  struct range *next;
};

template <class TYPE>
class range_list
{
public:
  // constructors & destructors.
  ~range_list();
  range_list();
  range_list(const char *list);
  range_list(range_list const &list);
  range_list &operator =(range_list const &list);

  // query functions.
  int valid() const;
  int empty() const;
  int member(TYPE id) const;
  
  // manipulation.
  void invalidate();
  range_list &merge  (range_list const &list);
  range_list &remove (range_list const &list);

  // output.
  int print(FILE *out) const;

private:  
  // Data members.
  range<TYPE> *head;
  int valid_flag;

  // Implementation.
  void destroy();
  static range<TYPE> *do_copy_list(range<TYPE> *);

  int clean(); // clean the range list eliminating overlaps etc.
};

#include "sid_list.cc"
// #ifdef CONFIG_COMPLETE_TEMPLATES
#include "sl-merge.cc"
// #endif

#endif /* __SID_LIST_H__ */
	
/* Local variables: */
/* mode: c++ */
/* End: */


