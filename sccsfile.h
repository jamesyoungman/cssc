/*
 * sccsfile.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Definition of the class sccs_file.
 *
 * @(#) CSSC sccsfile.h 1.2 93/11/13 00:11:17
 *
 */
 
#ifndef __SCCSFILE_H__
#define __SCCSFILE_H__

#include "sccsname.h"
#include "sid.h"
#include "sccsdate.h"
#include "list.h"
#include "linebuf.h"
#include "rel_list.h"

class sccs_pfile;	/* pfile.h */
class seq_state;	/* seqstate.h */

class sccs_file {
public:
	enum _mode { READ, UPDATE, CREATE };

private:
	struct delta {
		unsigned inserted, deleted, unchanged;
		char type;
		sid id;
		sccs_date date;
		mystring user;
		seq_no seq, prev_seq;
		list<seq_no> included, excluded, ignored;
		list<mystring> mrs;
		list<mystring> comments;

		delta() {}
		delta(char t, sid i, sccs_date d, mystring u, seq_no s, seq_no p,
		      list<mystring> ms, list<mystring> cs)
			: type(t), id(i), date(d), user(u),
			  seq(s), prev_seq(p), mrs(ms), comments(cs) {}
		delta(char t, sid i, sccs_date d, mystring u, seq_no s, seq_no p,
		      list<seq_no> incl, list<seq_no> excl,
		      list<mystring> ms, list<mystring> cs)
			: type(t), id(i), date(d), user(u),
			  seq(s), prev_seq(p), included(incl), excluded(excl), 
			  mrs(ms), comments(cs) {}

		struct delta &operator =(struct delta const &);

	private:
		delta(struct delta const &); /* undefined */
	};

	class _delta_table: public list<struct delta> {
		int *seq_table;
		seq_no high_seqno;
		sid high_release;

		void build_seq_table();
		void update_highest(struct delta const &delta);

		_delta_table &operator =(_delta_table const &); /* undefined */
		_delta_table(_delta_table const &); /* undefined */

	public:
		_delta_table(): seq_table(NULL), high_seqno(0),
		                high_release(NULL) {}

		void add(struct delta const &delta);		
		void prepend(struct delta const &delta); /* sf-add.c */

		struct delta const &
		operator [](seq_no seq) {
			assert(seq > 0 && seq <= high_seqno);
			if (seq_table == NULL) {
				build_seq_table();
			}
			return select(seq_table[seq]);
		}

		struct delta const *find(sid id) const; 

		seq_no highest_seqno() const { return high_seqno; }
		sid highest_release() const { return high_release; }

		~_delta_table() {
			if (seq_table != NULL) {
				free(seq_table);
			}
		}
	};

	class delta_iterator {
		class _delta_table const &dtbl;
		int pos;

	public:
		delta_iterator(class _delta_table const &d)
			: dtbl(d), pos(-1) {}

#pragma warn -inl

		int
		next(int all = 0) {
			while (++pos < dtbl.length()) {
				if (all || dtbl.select(pos).type == 'D') {
					return 1;
				}
			}
			return 0;
		}

#pragma warn .inl

		int index() const { return pos; }

		struct delta const *
		operator ->() const {
			return &dtbl.select(pos);
		}

		void rewind() { pos = -1; }
	};

	sccs_name &name;
	FILE *f;
	enum _mode mode;
	class _linebuf linebuf;
	int lineno;
	long body_offset;
	int body_lineno;

	class _delta_table delta_table;
	list<mystring> users;
	struct sccs_file_flags {
		mystring type;
		mystring mr_checker;
	  	int no_id_keywords_is_fatal;
		int branch;
		mystring module;
		release floor;
		release ceiling;
		sid default_sid;
		int null_deltas;
		int joint_edit;
		release_list locked;
		int all_locked;
		mystring user_def;
		mystring reserved;
	  	int encoded;
	} flags;
	list<mystring> comments;

	static FILE *open_sccs_file(const char *name, enum _mode mode,
				    unsigned *sump);
	NORETURN corrupt(const char *why) const POSTDECL_NORETURN;
	void check_arg() const;
	void check_noarg() const;
	unsigned short strict_atous(const char *s) const;
	
	int
	read_line_param(FILE *f) {
		if (linebuf.read_line(f)) {
			return 1;
		}
		linebuf[strlen(linebuf) - 1] = '\0';
		return 0;
	}

	int read_line();
	void read_delta();
	void seek_to_body();

	mystring get_module_name() const;

public:
	sccs_file(sccs_name &name, enum _mode mode);

	sid find_most_recent_sid(sid id) const;
        bool find_most_recent_sid(sid& s, sccs_date& d) const;

	int
	is_delta_creator(const char *user, sid id) const {
		struct delta const *delta = delta_table.find(id);
		return delta != NULL && strcmp(delta->user, user) == 0;
	}

	~sccs_file();

	/* sf-get.c */
private:
	struct subst_parms {
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

	void prepare_seqstate(seq_state &state, seq_no seq);

	typedef int (sccs_file::*subst_fn_t)(const char *,
					     struct subst_parms *) const;

	void get(mystring name, class seq_state &state,
		 struct subst_parms &parms,
#ifdef __GNUC__
		 subst_fn_t subst_fn = 0,
#else
		 int (sccs_file::*subst_fn)(const char *,
					    struct subst_parms *) const = 0,
#endif
		 int show_sid = 0, int show_module = 0, int debug = 0);

	/* sf-get2.c */
	int write_subst(const char *start, struct subst_parms *parms) const;

public:
	struct get_status {
		unsigned lines;
		list<sid> included, excluded;
	};

	bool find_requested_sid(sid requested, sid &found) const ;
	sid find_next_sid(sid requested, sid got, int branch,
			  sccs_pfile &pfile) const;
	void test_locks(sid got, sccs_pfile &pfile) const;

	struct get_status get(FILE *out, mystring name, sid id,
			      sccs_date cutoff = NULL,
#ifdef __GNUC__
			      sid_list include = sid_list(""),
			      sid_list exclude = sid_list(""),
#else
			      sid_list include = sid_list(""), sid_list exclude = sid_list(""),
#endif
			      int keywords = 0, const char *wstring = NULL,
			      int show_sid = 0, int show_module = 0,
			      int debug = 0);


	/* sf-get3.c */
private:
	void prepare_seqstate(seq_state &state, sid_list include,
			      sid_list exclude, sccs_date cutoff);

	/* sf-chkid.c */

	static int check_id_keywords(const char *s);

	/* sf-write.c */
private:
	NORETURN xfile_error(const char *msg) const POSTDECL_NORETURN;
	FILE *start_update() const;
	int write_delta(FILE *out, struct delta const &delta) const;
	int write(FILE *out) const;
	void end_update(FILE *out) const;

public:
	static void update_checksum(const char *name);
	void update();

	/* sf-add.c */

	FILE *start_update(struct delta const &new_delta);
	void end_update(FILE *out, struct delta const &new_delta);

	/* sf-delta.c */
private:
	void check_keywords_in_file(const char *name);

public:
	int
	mr_required() const {
		return flags.mr_checker;
	}

 	int check_mrs(list<mystring> mrs);

	void add_delta(mystring gname, sccs_pfile &pfile,
		       list<mystring> mrs, list<mystring> comments);

	/* sf-admin.c */

	void admin(const char *file_comment,
		   list<mystring> set_flags, list<mystring> unset_flags,
		   list<mystring> add_users, list<mystring> erase_users);
	void create(release first_release, const char *iname,
		    list<mystring> mrs, list<mystring> comments,
		    int suppress_comments);

	/* sf-prs.c */
private:
	void get(FILE *out, mystring name, seq_no seq);
	void print_flags(FILE *out) const;
	void print_delta(FILE *out, const char *format,
			 struct delta const &delta);

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
  

	void prs(FILE *out, mystring format, sid rid, sccs_date cutoff,
	         enum when when, int all_deltas);

  void prt(FILE *out, struct cutoff exclude, int all_deltas,
	   //
	   int print_body, int print_delta_table, int print_flags,
	   int incl_excl_ignore, int first_line_only, int print_desc,
	   int print_users) const;
  
	/* sf-cdc.c */

	void cdc(sid id, list<mystring> mrs, list<mystring> comments);

	/* sf-rmdel.c */

	void rmdel(sid rid);

};

/* l-split.c */

list<mystring> split_mrs(mystring mrs);
list<mystring> split_comments(mystring comments);

#endif /* __SCCSFILE_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
