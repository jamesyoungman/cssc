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
#include "run.h"
#include "linebuf.h"
#include "delta.h"
#include "delta-table.h"
#include "bodyio.h"
#include "filediff.h"


// We use @LIBOBJS@ instead now.
// #ifndef HAVE_STRSTR
// #include "strstr.cc"
// #endif

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-delta.cc,v 1.18 1998/02/12 21:33:31 james Exp $";
#endif

class diff_state
{
public:
	enum state { START, NOCHANGE, DELETE, INSERT, END };

private:
	enum state _state;
	int in_lineno, out_lineno;
	int lines_left;
	int change_left;

	FILE *in;
	cssc_linebuf linebuf;

	NORETURN corrupt() POSTDECL_NORETURN;
	NORETURN corrupt(const char *msg) POSTDECL_NORETURN;

	void next_state();
	int read_line() { return linebuf.read_line(in); }

public:
  diff_state(FILE *f)
    : _state(START), 
      in_lineno(0), out_lineno(0),
      lines_left(0), change_left(0), in(f)
    {
    }

	enum state process(FILE *out, seq_no seq);

	const char *
	get_insert_line() {
		ASSERT(_state == INSERT);
		ASSERT(linebuf[0] == '>' && linebuf[1] == ' ');
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
diff_state::corrupt(const char *msg)
{
  quit(-1, "Diff output corrupt. (%s)", msg);
}


/* Figure out what the new state should be by processing the
   diff output. */

inline void
diff_state::next_state()
{
  if (_state == DELETE && change_left != 0)
    {
      if (read_line())
	{
	  corrupt();
	}
      if (strcmp(linebuf.c_str(), "---\n") != 0)
	{
	  corrupt("---");
	}
      lines_left = change_left;
      change_left = 0;
      _state = INSERT;
      return;
    }

  if (_state != NOCHANGE)
    {
      if (read_line())
	{
	  if (ferror(in))
	    {
	      corrupt();
	    }
	  _state = END;
	  return;
	}

      /* Ignore "\ No newline at end of file" if it appears
	 at the end of the diff output. */

      if (linebuf[0] == '\\')
	{
	  if (!read_line())
	    {
	      corrupt("Expected EOF");
	    }
	  if (ferror(in))
	    {
	      corrupt();
	    }
	  _state = END;
	  return;
	}
			
    }

  char *s = NULL;
  int line1, line2, line3, line4;
  char c;

  line1 = (int) strtol(linebuf.c_str(), &s, 10);
  line2 = line1;
  if (*s == ',')
    {
      line2 = (int) strtol(s + 1, &s, 10);
      if (line2 <= line1)
	{
	  corrupt("left end line");
	}
    }

  c = *s;

  ASSERT(c != '\0');
		
  if (c == 'a')
    {
      if (line1 >= in_lineno)
	{
	  _state = NOCHANGE;
	  lines_left = line1 - in_lineno + 1;
	  return;
	}
      if (line1 + 1 != in_lineno)
	{
	  corrupt("left start line [case 1]");
	}
    }
  else
    {
      if (line1 > in_lineno)
	{
	  _state = NOCHANGE;
	  lines_left = line1 - in_lineno;
	  return;
	}
      if (line1 != in_lineno)
	{
	  corrupt("left start line [case 2]");
	}
    }

  line3 = (int) strtol(s + 1, &s, 10);
  if (c == 'd')
    {
      if (line3 != out_lineno)
	{
	  corrupt("right start line [case 1]");
	}
    }
  else
    {
      if (line3 != out_lineno + 1)
	{
	  corrupt("right start line [case 2]");
	}
    }
  
  line4 = line3;
  if (*s == ',')
    {
      line4 = (int) strtol(s + 1, &s, 10);
      if (line4 <= line3)
	{
	  corrupt("right end line");
	}
    }

  if (*s != '\n')
    {
      corrupt("EOL");
    }

  switch(c)
    {
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
		ASSERT(lines_left >= 0);
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


/* Adds a new delta to the SCCS file.  It doesn't add the delta to the
   delta list in sccs_file object, so this should be the last operation
   performed before the object is destroyed. */

void
sccs_file::add_delta(mystring gname, sccs_pfile &pfile,
		     list<mystring> mrs, list<mystring> comments) {
	ASSERT(mode == UPDATE);

	check_keywords_in_file(gname.c_str());


	/*
	 * At this point, encode the contents of "gname", and pass
	 * the name of this encoded file to diff, instead of the 
	 * name of the binary file itself.
	 */
	mystring file_to_diff;
	if (flags.encoded)
	  {
	    mystring uname(name.sub_file('u'));
	    encode_file(gname.c_str(), uname.c_str());
	    file_to_diff = uname;
	  }
	else
	  {
	    file_to_diff = gname;
	  }
	
	seq_state sstate(highest_delta_seqno());
	const delta *got_delta = find_delta(pfile->got);
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

	mystring dname(name.sub_file('d'));
	FILE *get_out = fopen(dname.c_str(), "wt");
	if (NULL == get_out)
	    quit(-1, "Cannot open file %s", dname.c_str());
	

	struct subst_parms parms(get_out, NULL, delta(), 
				 0, sccs_date(NULL));
	seq_state gsstate = sstate;
	get(dname, gsstate, parms, 0, 0, 0, 0, GET_NO_DECODE);

	if (fclose_failed(fclose(get_out)))
	  quit(errno, "Failed to close temporary file");
	
	FileDiff differ(dname.c_str(), file_to_diff.c_str());
	FILE *diff_out = differ.start();
	
	class diff_state dstate(diff_out);

	seek_to_body();

	list<seq_no> included, excluded;
	seq_no seq;
	for(seq	= 1; seq < highest_delta_seqno(); seq++) {
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
	    
	    ASSERT(id.valid());
	    
	    int infinite_loop_escape = 10000;

	    while (null_rel < last_auto_rel)
	      {
		ASSERT(id.valid());
		// add a new automatic "null" release.  Use the same
		// MRs as for the actual delta (is that right?) but
		seq_no new_seq = delta_table->next_seqno();
		
		// Set up for adding the next release.
		id = release(null_rel);
		id.next_level();
		
		delta null_delta('D', id, sccs_date::now(),
				 get_user_name(), new_seq, predecessor_seq,
				 mrs, auto_comment);
		null_delta.inserted = 0;
		null_delta.deleted = 0;
		null_delta.unchanged = 0;

		delta_table->prepend(null_delta);
		
		predecessor_seq = new_seq;

		++null_rel;

		--infinite_loop_escape;
		ASSERT(infinite_loop_escape > 0);
	      }
	  }
	seq_no new_seq = delta_table->next_seqno();
	delta new_delta('D', pfile->delta, sccs_date::now(),
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
			seq_no seq = strict_atous(plinebuf->c_str() + 3);

			if (seq < 1 || seq > highest_delta_seqno()) {
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
					ASSERT(c != -1);
#ifdef DEBUG_FILE
					fprintf(df, "%4d %4d - %s\n",
						dstate.in_line(),
						dstate.out_line(),
					        linebuf.c_str());
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
					ASSERT(c != -1);
#ifdef DEBUG_FILE
					fprintf(df, "%4d %4d   %s\n",
						dstate.in_line(),
						dstate.out_line(),
						linebuf.c_str());
					fflush(df);
#endif
					new_delta.unchanged++;
					break;

				default:
					abort();
				}

			} while (action == diff_state::INSERT);
		}

		if (c == -1) {
			break;
		}

		fputs(plinebuf->c_str(), out);
		putc('\n', out);
	}

#ifdef DEBUG_FILE
	fclose(df);
#endif


	pfile.delete_lock();
	pfile.update();

	end_update(out, new_delta);

	differ.finish();
	diff_out = 0;
	if (0 != remove(dname.c_str()))
	  {
	    perror(dname.c_str()); // TODO: should we quit?
	  }
	
	// If we had created a temporary file, delete it now.
	// We should probably not exit fatally if we fail to
	// unlink the temporary file.
	if (flags.encoded)
	  {
	    if (0 != remove(file_to_diff.c_str()))
	      {
		perror(file_to_diff.c_str());
	      }
	  }
	
	new_delta.id.print(stdout);
	printf("\n"
	       "%lu inserted\n%lu deleted\n%lu unchanged\n",
	       new_delta.inserted, new_delta.deleted, new_delta.unchanged);
}

/* Local variables: */
/* mode: c++ */
/* End: */
