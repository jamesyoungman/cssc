/*
 * sccsfile.h: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2007,
 *  2008, 2009, 2010, 2011, 2014, 2019 Free Software Foundation, Inc.
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
 * Definition of the class sccs_file.
 */

#ifndef CSSC__SCCSFILE_H__
#define CSSC__SCCSFILE_H__

#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include "sccsname.h"
#include "sid.h"
#include "sccsdate.h"
#include "rel_list.h"
#include "delta.h"
#include "delta-iterator.h"
#include "pfile.h"
#include "mode.h"
#include "optional.h"
#include "parser.h"

class seq_state;        /* seqstate.h */
class cssc_linebuf;
class FilePosSaver;             // filepos.h

struct delta;
class cssc_delta_table;
class delta_iterator;

struct get_status
{
  unsigned lines;
  std::vector<sid> included, excluded;
};

class sccs_file
{
public:
  // sccs_file::sccs_file(sccs_name&, enum _mode) MUST
  // take a non-const reference to an sccs_name as an
  // argument in order to get the semantics of lock
  // ownership correct; an sccs_name carries with it a
  // lock, so if we copy it, either the copy does not
  // have a lock or we have too many locks in total.
  sccs_file(sccs_name &name, sccs_file_open_mode mode,
	    ParserOptions = ParserOptions());
  ~sccs_file();

  enum class when { EARLIER, SIDONLY, LATER };
  enum { GET_NO_DECODE = true };

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
    cssc::Failure print(FILE *out) const;
  };

  // sccs_file::get performs the get operation for the "get" binary.
  cssc::FailureOr<get_status> get(FILE *out,
				  const std::string& gname,
				  FILE *summary_file,
				  sid id,
				  sccs_date cutoff_date,
				  sid_list include,
				  sid_list exclude,
				  bool keywords,
				  cssc::optional<std::string> wstring,
				  bool show_sid, bool show_module,
				  bool debug, bool for_edit);

  // do_get emits the gotten body (i.e. the actual result you would
  // get from "get -p s.foo").  It's used by prs, delta and so forth,
  // as well as sccs_file::get().
  cssc::Failure do_get(const std::string& gname, class seq_state &state,
		       struct subst_parms &parms,
		       bool do_kw_subst,
		       int show_sid, int show_module, int debug,
		       bool no_decode, bool for_edit);

  // Note: add_delta will succeed even for users not in the authorized
  // user list.  If you want the authorized user list to be checked,
  // call authorised().
  // TODO: return cssc::Failure instead of bool?
  bool add_delta(const std::string& gname,
		 sccs_pfile &pfile,
		 sccs_pfile::iterator it,
                 const std::vector<std::string>& mrs, const std::vector<std::string>& comments,
                 bool display_diff_output);

  // TODO: return cssc::Failure instead of bool?
  bool admin(const char *file_comment,
             bool force_binary,
             const std::vector<std::string>& set_flags, const std::vector<std::string>& unset_flags, // FIXME: consider something more efficient
             const std::vector<std::string>& add_users,
	     const std::unordered_set<std::string>& erase_users);
  // TODO: return cssc::Failure instead of bool?
  bool create(const sid &initial_sid,
              const char *iname,
              const std::vector<std::string>& mrs,
              std::vector<std::string>* comments,
              int suppress_comments,
              bool force_binary);

  // Print information about deltas.  Return true if the search
  // criteria matched a delta.
  cssc::FailureOr<bool> prs(FILE *out, const char *outname,
			    const std::string& format, sid rid, sccs_date cutoff_date,
			    enum when when, delta_selector selector);

  cssc::Failure prt(FILE *out, struct cutoff exclude, delta_selector selector,
		    int print_body, int print_delta_table, int print_flags,
		    int incl_excl_ignore, int first_line_only, int print_desc,
		    int print_users) const;

  // The caller must check edit_mode_permitted() before calling cdc().
  void cdc(delta*, const std::vector<std::string>& mrs, const std::vector<std::string>& comments);
  cssc::Failure rmdel(sid rid);
  // TODO: return cssc::Failure instead of bool?
  bool validate() const;

  ////////////////////////////////////////////////////////////////////////

  bool checksum_ok() const;

  sid find_most_recent_sid(sid id) const;
  // TODO: return cssc::Failure instead of bool?
  bool find_most_recent_sid(sid& s, sccs_date& d) const;

  bool is_delta_creator(const char *user, sid id) const;

  void check_bk_flag(const sccs_file_location&, char flagchar) const;

  /* Forwarding functions for the delta table.
   */
  const delta *find_delta(sid id) const;
  const delta *find_any_delta(sid id) const;
  delta *find_delta(sid id);
  seq_no highest_delta_seqno() const;
  sid highest_delta_release() const;
  sid seq_to_sid(seq_no) const;


  /* sf-get.c */
  // TODO: return cssc::FailureOr<sid> instead? Or an optional<sid>?
  bool find_requested_sid(sid requested, sid &found,
                          bool include_branches=false) const ;
  // TODO: return cssc::FailureOr<sid> instead?  Or an optional<sid>?
  bool find_requested_seqno(seq_no n, sid &found) const ;
  sid find_next_sid(sid requested, sid got, int branch,
                    const sccs_pfile &pfile, int *failed) const;
  // TODO: return cssc::Failure instead of bool?
  bool test_locks(sid got, const sccs_pfile&) const;


  // TODO: return cssc::Failure instead of bool?
  bool update_checksum();
  // TODO: return cssc::Failure instead of bool?
  bool update();

  /* sf-add.c */

  cssc::FailureOr<FILE*> start_update(struct delta const &new_delta);
  cssc::Failure end_update(FILE **out, struct delta const &new_delta);

  int mr_required() const
  {
    if (flags.mr_checker)
      return 1;
    else
      return 0;
  }

  // TODO: return cssc::Failure instead of bool?
  bool check_mrs(const std::vector<std::string>& mrs);

  /* sccsfile.cc */
  void set_mr_checker_flag(const char *s);
  void set_module_flag(const char *s);
  void set_user_flag(const char *s);
  void set_reserved_flag(const char *s);
  void set_expanded_keyword_flag(const char *s);
  void set_type_flag(const char *s);
  bool gfile_should_be_executable() const;


  /* Used by get.cc (implemented in sccsfile.cc) */
  bool branches_allowed() const;

  /* val.cc */
  const std::string  get_module_type_flag();

  /* sf-admin.c */
  std::string get_module_name() const;

  // TODO: return cssc::Failure instead of bool?
  bool validate_seq_lists(const delta_iterator& d) const;
  // TODO: return cssc::Failure instead of bool?
  bool validate_isomorphism() const;
  // TODO: return cssc::Failure instead of bool?
  bool check_loop_free(cssc_delta_table* t,
		       seq_no starting_seq,
		       std::vector<bool>& loopfree,
		       std::vector<bool>& seen) const;
  /* Support for BitKeeper files */
  cssc::Failure edit_mode_permitted(bool editing) const;

  // Is the current user in the authorized user list?
  bool authorised() const;

  // Implementation is protected; in the existing [MySC]
  // implementation some of the implementation is private where
  // it might better be protected.
protected:
  bool sid_in_use(sid id, const sccs_pfile& p) const;

  /* sf-get3.c */
  void prepare_seqstate(seq_state &state, seq_no seq,
                        sid_list include,
                        sid_list exclude, sccs_date cutoff_date);
  void prepare_seqstate_1(seq_state &state, seq_no seq);
  void prepare_seqstate_2(seq_state &state, sid_list include,
                        sid_list exclude, sccs_date cutoff_date);

  /* sf-write.c */
  void xfile_error(const char *msg) const;
  cssc::FailureOr<FILE*> start_update();         // this opens the x-file
  cssc::Failure write_delta(FILE *out, struct delta const &delta) const;
  cssc::Failure write(FILE *out) const;
  // TODO: return cssc::Failure instead of bool?
  cssc::Failure end_update(FILE **out);  // NB: this closes the x-file too.
  cssc::Failure rehack_encoded_flag(FILE *out, int *sum) const;

private:
  /* sf-prs.c */
  cssc::Failure print_flags(FILE *out) const;
  cssc::Failure print_delta(FILE *out, const char *outname, const char *format,
			    struct delta const &delta);
  // Print a single key (e.g. :W:) from the prs format string.  On
  // success, if the result is true, the key was known.  If false, not
  // known.
  cssc::FailureOr<bool> print_delta_key(FILE *out, const char *outname,
					unsigned key,
					struct delta const &delta);

  /* sf-kw.cc */
  void no_id_keywords(const char name[]) const;

  cssc::Failure check_keywords_in_file(const char *name);

  // Because we now have a pointer member, don't use the compiler's
  // default assignment and constructor.
  const sccs_file& operator=(const sccs_file&) = delete; // not allowed to use!
  sccs_file(const sccs_file&) = delete;  // not allowed to use!

  cssc::Failure print_subsituted_flags_list(FILE *out, const char* separator) const;
  static bool is_known_keyword_char(char c);

  cssc::FailureOr<bool> emit_keyletter_expansion(FILE *out, struct subst_parms *parms, const delta& d, char c) const;
  cssc::Failure write_subst(const char *start,
			    struct subst_parms *parms,
			    struct delta const& gotten_delta,
			    bool force_expansion) const;

  bool sid_matches(const sid& requested,
		   const sid& found,
		   bool get_top_delta) const;

  NORETURN corrupt_file(const char *fmt, ...) const POSTDECL_NORETURN;

  void set_sfile_executable(bool state);
  bool sfile_should_be_executable() const;

  struct sccs_file_flags
  {
    // TODO: consider std::unique_ptr<std::string> instead of std::string*.
    std::string *type;
    std::string *mr_checker;
    int no_id_keywords_is_fatal;
    int branch;
    std::string *module;
    release floor;
    release ceiling;
    sid default_sid;
    int null_deltas;
    int joint_edit;
    release_list locked;
    int all_locked;
    std::string *user_def;
    std::string *reserved;

    int encoded;
    int executable;
    std::set<char> substitued_flag_letters; // "y" flag (Solaris 8 only)
  } flags;

  sccs_name& name;
  bool checksum_valid_;
  enum sccs_file_open_mode mode;
  bool xfile_created;
  bool edit_mode_ok_;
  bool sfile_executable;
  std::unique_ptr<cssc_delta_table> delta_table;
  std::unique_ptr<sccs_file_body_scanner> body_scanner_;
  std::vector<std::string> users;	// FIXME: consider something more efficient.
  std::vector<std::string> comments;
};

/* sf-prt.cc */
cssc::Failure print_flag(FILE *out, const char *fmt,  release flag, int& count);
cssc::Failure print_flag(FILE *out, const char *fmt, std::string flag, int& count);
cssc::Failure print_flag(FILE *out, const char *fmt,  int flag, int& count);
cssc::Failure print_flag(FILE *out, const char *fmt,  sid flag, int& count);

/* sf-prs.cc */
cssc::Failure print_flag2(FILE *out, const char *s, const sid& it);
cssc::Failure print_flag2(FILE *out, const char *s, const release_list& it);
cssc::Failure print_flag2(FILE *out, const char *s, const release& it);

/* l-split.c */

std::vector<std::string> split_mrs(const std::string& mrs);
std::vector<std::string> split_comments(const std::string& comments);

#endif /* CSSC__SCCSFILE_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
