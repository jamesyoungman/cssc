/*
 * sf-delta.c: Part of GNU CSSC.
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
 * Members of sccs_file for adding a delta to the SCCS file.
 *
 */

#include "cssc.h"
#include "sccsfile.h"
#include "pfile.h"
#include "seqstate.h"
#include "pipe.h"
#include "run.h"
#include "linebuf.h"

#ifndef HAVE_STRSTR
#include "strstr.cc"
#endif

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-delta.cc,v 1.10 1997/07/07 21:24:26 james Exp $";
#endif

class diff_state {
public:
	enum state { START, NOCHANGE, DELETE, INSERT, END };

private:
	enum state _state;
	int in_lineno, out_lineno;
	int lines_left;
	int change_left;

	FILE *in;
	class _linebuf linebuf;
	mystring gname;

	NORETURN corrupt() POSTDECL_NORETURN;
	NORETURN corrupt(const char *msg) POSTDECL_NORETURN;

	void next_state();
	int read_line() { return linebuf.read_line(in); }

public:
	diff_state(FILE *f, mystring name)
		: _state(START), 
		  in_lineno(0), out_lineno(0),
		  lines_left(0), change_left(0), in(f), gname(name) {}

	enum state process(FILE *out, seq_no seq);

	const char *
	get_insert_line() {
		assert(_state == INSERT);
		assert(linebuf[0] == '>' && linebuf[1] == ' ');
		return linebuf + 2;
	}

	int in_line() { return in_lineno; }
	int out_line() { return out_lineno; }
};


/* Quit with an appropriate error message when a read operation
   on the diff output fails. */

NORETURN
diff_state::corrupt() {
	if (ferror(in)) {
		quit(errno, "(diff output): Read error.");
	}
	quit(-1, "(diff output): Unexpected EOF.");
}


/* Quit with a cryptic error message indicating that something
   is wrong with the diff output. */

NORETURN
diff_state::corrupt(const char *msg) {
	quit(-1, "Diff output corrupt. (%s)", msg);
}


/* Figure out what the new state should be by processing the
   diff output. */

inline void
diff_state::next_state() {
	if (_state == DELETE && change_left != 0) {
		if (read_line()) {
			corrupt();
		}
		if (strcmp(linebuf, "---\n") != 0) {
			corrupt("---");
		}
		lines_left = change_left;
		change_left = 0;
		_state = INSERT;
		return;
	}

	if (_state != NOCHANGE) {
		if (read_line()) {
			if (ferror(in)) {
				corrupt();
			}
			_state = END;
			return;
		}

		/* Ignore "\ No newline at end of file" if it appears
		   at the end of the diff output. */

		if (linebuf[0] == '\\') {
			if (!read_line()) {
				corrupt("Expected EOF");
			}
			if (ferror(in)) {
				corrupt();
			}
			_state = END;
			return;
		}
			
	}

	char *s = NULL;
	int line1, line2, line3, line4;
	char c;

	line1 = (int) strtol(linebuf, &s, 10);
	line2 = line1;
	if (*s == ',') {
		line2 = (int) strtol(s + 1, &s, 10);
		if (line2 <= line1) {
			corrupt("left end line");
		}
	}

	c = *s;

	assert(c != '\0');
		
	if (c == 'a') {
		if (line1 >= in_lineno) {
			_state = NOCHANGE;
			lines_left = line1 - in_lineno + 1;
			return;
		}
		if (line1 + 1 != in_lineno) {
			corrupt("left start line");
		}
	} else {
		if (line1 > in_lineno) {
			_state = NOCHANGE;
			lines_left = line1 - in_lineno;
			return;
		}
		if (line1 != in_lineno) {
			corrupt("left start line");
		}
	}

	line3 = (int) strtol(s + 1, &s, 10);
	if (c == 'd') {
		if (line3 != out_lineno) {
			corrupt("right start line");
		}
	} else {
		if (line3 != out_lineno + 1) {
			corrupt("right start line");
		}
	}

	line4 = line3;
	if (*s == ',') {
		line4 = (int) strtol(s + 1, &s, 10);
		if (line4 <= line3) {
			corrupt("right end line");
		}
	}

	if (*s != '\n') {
		corrupt("EOL");
	}

	switch(c) {
	case 'a':
		_state = INSERT;
		lines_left = line4 - line3 + 1;
		break;		

	case 'd':
		_state = DELETE;
		lines_left = line2 - line1 + 1;
		break;

	case 'c':
		_state = DELETE;
		lines_left = line2 - line1 + 1;
		change_left = line4 - line3 + 1;
		break;

	default:
		corrupt("unknown operation");
	}
}


/* Figure out whether a line is being inserted, deleted or left unchanged.
   Output new control lines accordingly. */

inline /* enum */ diff_state::state
diff_state::process(FILE *out, seq_no seq) {
	if (_state != INSERT) {
		in_lineno++;
	}

	if (_state != END) {
		assert(lines_left >= 0);
		if (lines_left == 0) {
			if (_state == DELETE || _state == INSERT) {
				fprintf(out, "\001E %d\n", seq);
			}
			next_state();
			if (_state == INSERT) {
				fprintf(out, "\001I %d\n", seq);
			} else if (_state == DELETE) {
				fprintf(out, "\001D %d\n", seq);
			}
		}
		lines_left--;
	}

	if (_state == DELETE) {
		if (read_line()) {
			corrupt();
		}
		if (linebuf[0] != '<' || linebuf[1] != ' ') {
			corrupt("<");
		}
	} else {
		if (_state == INSERT) {
			if (read_line()) {
				corrupt();
			}
			if (linebuf[0] != '>' || linebuf[1] != ' ') {
				corrupt(">");
			}
		}
		out_lineno++;
	}

	return _state;
}


/* Warns or quits if the new delta doesn't include any id keywords */

void
sccs_file::check_keywords_in_file(const char *name) {
	FILE *f = fopen(name, "r");
	if (f == NULL) {
		quit(errno, "%s: Can't open file for reading.", name);
	}

	while(!read_line_param(f)) {
		if (check_id_keywords(linebuf)) {
			fclose(f);
			return;
		}
	}

	if (flags.no_id_keywords_is_fatal)
	  {
	    quit(-1, "%s: No id keywords.", name);
	  }
	else
	  {
	    fprintf(stderr, "%s: Warning: No id keywords.\n", name);
	  }
	fclose(f);
}


/* Adds a new delta to the SCCS file.  It doesn't add the delta to the
   delta list in sccs_file object, so this should be the last operation
   performed before the object is destroyed. */

void
sccs_file::add_delta(mystring gname, sccs_pfile &pfile,
		     list<mystring> mrs, list<mystring> comments) {
	assert(mode == UPDATE);

	check_keywords_in_file(gname);

	seq_state sstate(delta_table.highest_seqno());
	const struct delta *got_delta = delta_table.find(pfile->got);
	if (got_delta == NULL)
	  {
	    quit(-1, "Locked delta doesn't exist!");
	  }

	// Remember seq number that will be the predecessor of the 
	// one for the delta.
	seq_no predecessor_seq = got_delta->seq;

	prepare_seqstate(sstate, predecessor_seq);
	prepare_seqstate(sstate, pfile->include, pfile->exclude,
			 sccs_date(NULL));

	Pipe diff_in;

	FILE *get_out = diff_in.write_stream();
	if (get_out != NULL) {
		struct subst_parms parms(get_out, NULL, delta(), 
					 0, sccs_date(NULL));
#ifndef USE_PIPE
		seq_state gsstate = sstate;
		get("diff pipe", gsstate, parms);
#else
		sccs_file file(name, READ);
		file.get("(diff pipe)", sstate, parms);
#endif
	}
	
	diff_in.write_close();
	
	Pipe diff_out;

	int ret;

#ifndef USE_PIPE

	ret = run_diff(gname, diff_in, diff_out);
	if (ret != STATUS(0) && ret != STATUS(1)) {
		quit(-1, CONFIG_DIFF_COMMAND ": Command failed.  "
			 STATUS_MSG(ret));
	}

	diff_in.read_close();

#else /* USE_PIPE */

	run_diff(gname, diff_in, diff_out);

	ret = diff_in.read_close();
	if (ret != STATUS(0)) {
		quit(-1, "get: Subprocess exited abnormally. "
		         STATUS_MSG(ret));
	}

#endif /* USE_PIPE */

	diff_out.write_close();

	class diff_state dstate(diff_out.read_stream(), gname);

	seek_to_body();

	list<seq_no> included, excluded;
	seq_no seq;
	for(seq	= 1; seq < delta_table.highest_seqno(); seq++) {
		if (sstate.is_explicit(seq)) {
			if (sstate.is_included(seq)) {
				included.add(seq);
			} else if (sstate.is_excluded(seq)) {
				excluded.add(seq);
			}
		}
	}

	// Create any required null deltas if required.
	if (flags.null_deltas)
	  {
	    // figure out how many null deltas to make to fill the gap
	    // between the highest current trunk release and the one
	    // belonging to the new delta.

	    // use our own comment.
	    list <mystring> auto_comment;
	    auto_comment.add(mystring("AUTO NULL DELTA"));
	    
	    release last_auto_rel = release(pfile->delta);
	    // --last_auto_rel;
	    
	    sid id(got_delta->id);
	    release null_rel = release(id);
	    ++null_rel;
	    
	    assert(id.valid());
	    
	    int infinite_loop_escape = 10000;

	    while (null_rel < last_auto_rel)
	      {
		assert(id.valid());
		// add a new automatic "null" release.  Use the same
		// MRs as for the actual delta (is that right?) but
		seq_no new_seq = delta_table.next_seqno();
		
		// Set up for adding the next release.
		id = release(null_rel);
		id.next_level();
		
		struct delta null_delta('D', id, sccs_date::now(),
					get_user_name(), new_seq, predecessor_seq,
					mrs, auto_comment);
		null_delta.inserted = 0;
		null_delta.deleted = 0;
		null_delta.unchanged = 0;

		delta_table.prepend(null_delta);
		
		predecessor_seq = new_seq;

		++null_rel;

		--infinite_loop_escape;
		assert(infinite_loop_escape > 0);
	      }
	  }
	seq_no new_seq = delta_table.next_seqno();
	struct delta new_delta('D', pfile->delta, sccs_date::now(),
			       get_user_name(), new_seq, predecessor_seq,
			       included, excluded, mrs, comments);
	new_delta.inserted = 0;
	new_delta.deleted = 0;
	new_delta.unchanged = 0;

	
	// Begin the update by writing out the new delta.
	FILE *out = start_update(new_delta);

	
#undef DEBUG_FILE
#ifdef DEBUG_FILE
	FILE *df = fopen("delta.dbg", "w");
#endif

	while(1) {
		int c = read_line();

		if (c != 0 && c != -1) {
			seq_no seq = strict_atous(linebuf + 3);

			if (seq < 1 || seq > delta_table.highest_seqno()) {
				corrupt("Invalid sequence number");
			}

			const char *msg = NULL;

			switch(c) {
			case 'E':
				msg = sstate.end(seq);
				break;

			case 'D':
			case 'I':
				msg = sstate.start(seq, c == 'I');
				break;

			default:
				corrupt("Unexpected control line");
				break;
			}

			if (msg != NULL) {
				corrupt(msg);
			}
		} else if (sstate.include_line()) {
			/* enum */ diff_state::state action;

			do {
				action = dstate.process(out, new_delta.seq);
				switch(action) {

				case diff_state::DELETE:
					assert(c != -1);
#ifdef DEBUG_FILE
					fprintf(df, "%4d %4d - %s\n",
						dstate.in_line(),
						dstate.out_line(),
					        (const char *) linebuf);
						fflush(df);
#endif
					new_delta.deleted++;
					break;

				case diff_state::INSERT:
#ifdef DEBUG_FILE
					fprintf(df, "%4d %4d + %s",
						dstate.in_line(),
						dstate.out_line(),
					        dstate.get_insert_line());
						fflush(df);
#endif
					new_delta.inserted++;
					fputs(dstate.get_insert_line(), out);
					break;

				case diff_state::END:
					if (c == -1) {
						break;
					}
					/* FALLTHROUGH */
				case diff_state::NOCHANGE:
					assert(c != -1);
#ifdef DEBUG_FILE
					fprintf(df, "%4d %4d   %s\n",
						dstate.in_line(),
						dstate.out_line(),
					        (const char *) 
						linebuf);
					fflush(df);
#endif
					new_delta.unchanged++;
					break;

				default:
					abort();
				}

			} while(action == diff_state::INSERT);
		}

		if (c == -1) {
			break;
		}

		fputs(linebuf, out);
		putc('\n', out);
	}

#ifdef DEBUG_FILE
	fclose(df);
#endif

#ifndef USE_PIPE
	diff_out.read_close();
#else
	ret = diff_out.read_close();
	if (ret != STATUS(0) && ret != STATUS(1)) {
		quit(-1, CONFIG_DIFF_COMMAND ": Command failed.  "
			 STATUS_MSG(ret));
	}
#endif

	pfile.delete_lock();
	pfile.update();

	end_update(out, new_delta);

	new_delta.id.print(stdout);
	printf("\n"
	       "%u inserted\n%u deleted\n%u unchanged\n",
	       new_delta.inserted, new_delta.deleted, new_delta.unchanged);
}

/* Local variables: */
/* mode: c++ */
/* End: */
