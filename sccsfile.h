/*
 * sccsfile.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998, Free Software Foundation, Inc. 
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
 * Definition of the class sccs_file.
 *
 * @(#) CSSC sccsfile.h 1.2 93/11/13 00:11:17
 *
 */
 
#ifndef CSSC__SCCSFILE_H__
#define CSSC__SCCSFILE_H__

#include "sccsname.h"
#include "sid.h"
#include "sccsdate.h"
#include "mylist.h"
// #include "linebuf.h"
#include "rel_list.h"

class sccs_pfile;	/* pfile.h */
class seq_state;	/* seqstate.h */
class cssc_linebuf;
class FilePosSaver;		// filepos.h

struct delta;
class cssc_delta_table;

class sccs_file
{
public:
  enum _mode { READ, UPDATE, CREATE };
  
private:
  
  sccs_name& name;
  FILE *f;
  bool checksum_valid;
  enum _mode mode;
  int lineno;
  long body_offset;
  int body_lineno;

  cssc_delta_table *delta_table;
  cssc_linebuf     *plinebuf;
  
  list<mystring> users;
  struct sccs_file_flags
  {
    mystring *type;
    mystring *mr_checker;
    int no_id_keywords_is_fatal;
    int branch;
    mystring *module;
    release floor;
    release ceiling;
    sid default_sid;
    int null_deltas;
    int joint_edit;
    release_list locked;
    int all_locked;
    mystring *user_def;
    mystring *reserved;
    
    int encoded;
  } flags;

  list<mystring> comments;

  static FILE *open_sccs_file(const char *name, enum _mode mode,
			      int *sump);
  NORETURN corrupt(const char *why) const POSTDECL_NORETURN;
  void check_arg() const;
  void check_noarg() const;
  unsigned short strict_atous(const char *s) const;
  unsigned long strict_atoul(const char *s) const;
  
  int read_line_param(FILE *f);

  int read_line();
  void read_delta();
  bool seek_to_body();

  mystring get_module_name() const;

public:

  // sccs_file::sccs_file(sccs_name&, enum _mode) MUST 
  // take a non-const reference to an sccs_name as an 
  // argument in order to get the semantics of lock
  // ownership correct; an sccs_name carries with it a
  // lock, so if we copy it, either the copy does not
  // have a lock or we have too many locks in total.
  sccs_file(sccs_name &name, enum _mode mode);
  bool checksum_ok() const;
  
  sid find_most_recent_sid(sid id) const;
  bool find_most_recent_sid(sid& s, sccs_date& d) const;

  int is_delta_creator(const char *user, sid id) const;


  /* Forwarding functions for the delta table.
   */
  const delta *find_delta(sid id) const; 
  const delta *find_any_delta(sid id) const; 
  delta *find_delta(sid id); 
  seq_no highest_delta_seqno() const;
  sid highest_delta_release() const;
  sid seq_to_sid(seq_no) const;
  

  /* Forwarding functinos for the line buffer.
   */
  char bufchar(int pos) const;
  
  ~sccs_file();

  /* sf-get.c */
private:
  struct subst_parms
  {
    FILE *out;
    const char *wstring;
    struct delta const &delta;
    unsigned out_lineno;
    sccs_date now;
    int found_id;
    
    subst_parms(FILE *o, const char *w, struct delta const &d,
		int l, sccs_date n)
      : out(o), wstring(w), delta(d), out_lineno(l), now(n),
	found_id(0) {}
  };

  bool prepare_seqstate(seq_state &state, seq_no seq);

  typedef int (sccs_file::*subst_fn_t)(const char *,
				       struct subst_parms *,
				       struct delta const&) const;

  bool get(mystring name, class seq_state &state,
	   struct subst_parms &parms,
	   subst_fn_t subst_fn = 0,
	   int show_sid = 0, int show_module = 0, int debug = 0,
	   bool no_decode = false);
  enum { GET_NO_DECODE = true };
  
  /* sf-get2.c */
  int write_subst(const char *start,
		  struct subst_parms *parms,
		  struct delta const& gotten_delta) const;

public:
  struct get_status
  {
    unsigned lines;
    list<sid> included, excluded;
    bool     success;
  };

  bool find_requested_sid(sid requested, sid &found,
			  bool include_branches=false) const ;
  sid find_next_sid(sid requested, sid got, int branch,
		    sccs_pfile &pfile) const;
  bool test_locks(sid got, sccs_pfile &pfile) const;
  
  struct get_status get(FILE *out, mystring name, sid id,
			sccs_date cutoff_date = NULL,
			sid_list include = sid_list(""),
			sid_list exclude = sid_list(""),
			int keywords = 0, const char *wstring = NULL,
			int show_sid = 0, int show_module = 0,
			int debug = 0);

  /* sf-get3.c */
private:
  bool prepare_seqstate(seq_state &state, sid_list include,
			sid_list exclude, sccs_date cutoff_date);

  /* sf-write.c */
private:
  void xfile_error(const char *msg) const;
  FILE *start_update() const;
  int write_delta(FILE *out, struct delta const &delta) const;
  int write(FILE *out) const;
  bool end_update(FILE *out) const;
  int rehack_encoded_flag(FILE *out, int *sum) const;

public:
  static bool update_checksum(const char *name);
  bool update();

  /* sf-add.c */

  FILE *start_update(struct delta const &new_delta);
  void end_update(FILE *out, struct delta const &new_delta);

  /* sf-delta.c */
private:
  bool check_keywords_in_file(const char *name);

public:
  int
  mr_required() const
  {
    if (flags.mr_checker)
      return 1;
    else
      return 0;
  }

  int check_mrs(list<mystring> mrs);

  bool add_delta(mystring gname, sccs_pfile &pfile,
		 list<mystring> mrs, list<mystring> comments);

  /* sccsfile.cc */
  void set_mr_checker_flag(const char *s);
  void set_module_flag(const char *s);
  void set_user_flag(const char *s);
  void set_reserved_flag(const char *s);
  void set_type_flag(const char *s);


  /* Used by get.cc (implemented in sccsfile.cc) */
  bool branches_allowed() const;
  
  /* sf-admin.c */
  bool admin(const char *file_comment,
	     bool force_binary,
	     list<mystring> set_flags, list<mystring> unset_flags,
	     list<mystring> add_users, list<mystring> erase_users);
  bool create(release first_release, const char *iname,
	      list<mystring> mrs, list<mystring> comments,
	      int suppress_comments, bool force_binary);

  /* sf-prs.c */
private:
  bool get(FILE *out, mystring name, seq_no seq);
  void print_flags(FILE *out) const;
  void print_delta(FILE *out, const char *format,
		   struct delta const &delta);

  /* sf-kw.cc */
  void no_id_keywords(const char name[]) const;
  
public:
  enum when { EARLIER, SIDONLY, LATER };
  struct cutoff
  {
    bool enabled;
    bool most_recent_sid_only;
    sid  cutoff_sid;
    const struct delta *cutoff_delta;
    sccs_date first_accepted;
    sccs_date last_accepted;
    
    cutoff();
    bool excludes_delta(sid, sccs_date, bool& stop_now) const;
    void print(FILE *out) const;
  };
  

  bool prs(FILE *out, mystring format, sid rid, sccs_date cutoff_date,
	   enum when when, int all_deltas);

  bool prt(FILE *out, struct cutoff exclude, int all_deltas,
	   //
	   int print_body, int print_delta_table, int print_flags,
	   int incl_excl_ignore, int first_line_only, int print_desc,
	   int print_users) const;
  
  /* sf-cdc.c */

  bool cdc(sid id, list<mystring> mrs, list<mystring> comments);

  /* sf-rmdel.c */

  bool rmdel(sid rid);

  // Implementation is protected; in the existing [MySC]
  // implementation some of the implementation is private where
  // it might better be protected.
protected:

  // sid_in_use() should take a const sccs_pfile&, but iteration over
  // a sccs_pfile requires that it is not const (FIXME!).
  bool sid_in_use(sid id, sccs_pfile& p) const;

private:
  // Because we now have a pointer member, don't use the compiler's
  // default assignment and constructor.
  const sccs_file& operator=(const sccs_file&);	// not allowed to use!
  sccs_file(const sccs_file&);	// not allowed to use!
};

/* l-split.c */

list<mystring> split_mrs(mystring mrs);
list<mystring> split_comments(mystring comments);

#endif /* __SCCSFILE_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
