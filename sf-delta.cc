/*
 * sf-delta.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998,1999, Free Software Foundation, Inc. 
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
 * CSSC was originally based on MySC, by Ross Ridge, which was 
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
#include "delta-iterator.h"
#include "bodyio.h"
#include "filediff.h"
#include "err_no.h"

#undef JAY_DEBUG


#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-delta.cc,v 1.35 1999/05/16 16:53:17 james Exp $";
#endif

class FileDeleter
{
  mystring name;
  bool armed;
  
public:
  FileDeleter(const mystring& s) : name(s), armed(true) { }
  
  ~FileDeleter()
    {
      if (armed)
	{
	  const char *s = name.c_str();
	  if (0 != remove(s))
	    perror(s);
	}
    }
  void disarm() { armed = false; }
};



// Read some diff output.  Return the line number in the left file
// (i.e. the old version) where the next change takes place.
static int diff_interp(const char * s, char &type_indicator)
{
  int num;
  int dummy;
  char ch;
  
  if (3 == sscanf(s, "%d,%d%c", &num, &dummy, &ch))
    {
      type_indicator = ch;
      return num;
    }
  else if (2 == sscanf(s, "%d%c", &num, &ch))
    {
      type_indicator = ch;
      return num;
    }
  else
    {
      // unknown line format.
      type_indicator = '\0';
      return -1;
    }
}


// Read (and echo) lines from the body of the SCCS file, until we have 
// traversed exactly one line which is visible in the output, as specified
// by the passed-in seq_state object.
//
bool sccs_file::traverse_body_line(seq_state& sstate,
				   FILE *out,
				   bool bEOFisOK,
				   int * included_lines_traversed)
{
  int c;
  bool leading = true;
  bool bDebug = false;
  
  if (getenv("CSSC_DEBUG_TRAVERSE"))
    {
      bDebug = true;
    }
  
    
  // sccs_file::read_line() returns
  // 0 for a normal body line
  // >0 for a control line
  // <0 for EOF or error.

  // While "leading" is true, we have not yet read the included body line.
  // Once we traverse that line, we reset "leading".  When we read that body 
  // line, we push it back on the input (by remembering it, not by using 
  // stdio) and return;

  while (1)
    {
      c = read_line();
      
      if (bDebug)
	{
	  fprintf(stderr, "Traverse: c=%d\n", c);
	}

      if (c < 0)
	{
	  break;
	}
      
      if (c)			// control line.
	{
	  // we just read a body line.  Emit it.
	  if (bDebug)
	    {
	      fprintf(stderr, "Traverse: emitting body line %s\n",
		      plinebuf->c_str());
	    }
	  if (fprintf_failed(fprintf(out, "%s\n", plinebuf->c_str())))
	    {
	      return false;
	    }
	  
	  const seq_no seq = strict_atous(plinebuf->c_str() + 3);
	  if (seq < 1 || seq > highest_delta_seqno())
	    {
	      corrupt("Invalid sequence number");
	    }
	  
	  const char *msg = NULL;

	  // In the body part of the SCCS file, the only control lines 
	  // we expect are ^AI, ^AD, ^AE.
	  switch (c)
	    {
	    case 'E':
	      msg = sstate.end(seq);
	      break;
	      
	    case 'D':
	    case 'I':
	      msg = sstate.start(seq, c);
	      break;
	      
	    default:
	      corrupt("Unexpected control line");
	      break;
	    }
	  
	  if (msg != NULL)
	    {
	      corrupt(msg);
	    }
	}
      else if (leading)
	{
	  // we just read a body line.  Emit it.
	  if (bDebug)
	    {
	      fprintf(stderr, "Traverse: emitting body line %s\n",
		      plinebuf->c_str());
	    }
	  if (fprintf_failed(fprintf(out, "%s\n", plinebuf->c_str())))
	    {
	      return false;
	    }

	  // it is a body line.   If we are in insert mode, we have just 
	  // traversed an included body line, which is what we were 
	  // called to do.  We then switch to the mode where we eat
	  // the following control lines, until we find another body line.
	  if (1 /* sstate.include_line() */ )
	    {
	      if (bDebug)
		{
		  fprintf(stderr, "Traverse: got the line.\n",
			  plinebuf->c_str());
		}
	      leading = false;

	      if (sstate.include_line())
		{
		  ++(*included_lines_traversed);
		}
	    }
	}
      else
	{
	  if (bDebug)
	    {
	      fprintf(stderr, "Traverse: pushing back %s\n",
		      plinebuf->c_str());
	    }
	  push_back_current_line();
	  return true;
	}
    }

  if (ferror(f))
    {
      return false;
    }
  else  
    {
      // We reached EOF.
      // If we get EOF before consuming a body line, that is an error, 
      // unless we are just copying the trailing part of the SCCS file,
      // in which case EOF is harmless.
      if (!leading)
	{
	  return true;
	}
      else
	{
	  if (!bEOFisOK)
	    corrupt("Unexpected EOF while reading body lines");
	  return false;
	}
    }
}

bool
sccs_file::delta_do_insert(FILE *out,
			   cssc_linebuf& delta_buf,
			   struct delta& new_delta,
			   seq_no new_seq,
			   FILE * diff_out,
			   bool display_diff_output)
{
  if (fprintf_failed(fprintf(out, "\001I %u\n", (unsigned)new_seq)))
    {
      return false;
    }
  // Now read the lines which the diff output tells us we should 
  // insert.  These lines all begin with "> ".
  while (0 == delta_buf.read_line(diff_out))
    {
      const char * pdiff = delta_buf.c_str();
      if (display_diff_output)
	(void)printf("%s", pdiff);
      
      pdiff = delta_buf.c_str();
      if (pdiff[0] == '>' && pdiff[1] == ' ')
	{
	  if (fprintf_failed(fprintf(out, "%s", &pdiff[2])))
	    {
	      return false;
	    }
	  ++new_delta.inserted;
	}
      else
	{
	  // reached end of lines to insert.  Look for next 
	  // instruction.
	  break;
	}
    }
  if (fprintf_failed(fprintf(out, "\001E %u\n", (unsigned)new_seq)))
    {
      return false;
    }
}


bool
sccs_file::delta_do_delete(FILE *out,
			   cssc_linebuf& delta_buf,
			   struct delta& new_delta,
			   seq_no new_seq,
			   FILE * diff_out,
			   bool display_diff_output,
			   seq_state& sstate,
			   unsigned long int& next_body_line)
{
  
  if (fprintf_failed(fprintf(out, "\001D %u\n",
			     (unsigned)new_seq)))
    {
      return false;
    }
  while (0 == delta_buf.read_line(diff_out))
    {
      const char * pline = delta_buf.c_str();
      if (display_diff_output)
	(void)printf("%s", pline);
      
      if (pline[0] == '<' && pline[1] == ' ')
	{
	  // Don't emit the deleted line here, traverse_body_line() 
	  // will do that for us.
	  int increment = 0;
	  if (!traverse_body_line(sstate, out, false, &increment))
	    {
	      return false; // I/O failure.
	    }
	  next_body_line += increment;
	  new_delta.deleted += increment;
	}
      else
	{
	  // reached end of lines to delete.  Look for next 
	  // instruction.
	  break;
	}
    }
  if (fprintf_failed(fprintf(out, "\001E %u\n",
			     (unsigned)new_seq)))
    {
      return false;
    }
}

bool 
sccs_file::delta_do_change(FILE *out,
			   cssc_linebuf& delta_buf,
			   struct delta& new_delta,
			   seq_no new_seq,
			   FILE * diff_out,
			   bool display_diff_output,
			   seq_state& sstate,
			   unsigned long int& next_body_line)
{
  const char *pdiff = delta_buf.c_str();

  if (!delta_do_delete(out, delta_buf, new_delta, new_seq,
		       diff_out, display_diff_output,
		       sstate, next_body_line))
    {
      return false;
    }
  else
    {
      pdiff = delta_buf.c_str();
      if (pdiff[0] != '-')
	{
	  fatal_quit(-1,
		     "(diff output): Expected '---', got '%s'",
		     pdiff);
	}
      return delta_do_insert(out, delta_buf, new_delta, new_seq, diff_out,
			     display_diff_output);
    }
}

/* Adds a new delta to the SCCS file.  It doesn't add the delta to the
   delta list in sccs_file object, so this should be the last operation
   performed before the object is destroyed. */

bool
sccs_file::add_delta(mystring gname, sccs_pfile &pfile,
		     mylist<mystring> mrs, mylist<mystring> comments,
		     bool display_diff_output)
{
  ASSERT(mode == UPDATE);

  if (!check_keywords_in_file(gname.c_str()))
    return false;


  /*
   * At this point, encode the contents of "gname", and pass
   * the name of this encoded file to diff, instead of the 
   * name of the binary file itself.
   */
  mystring file_to_diff;
  if (flags.encoded)
    {
      mystring uname(name.sub_file('u'));
      if (0 != encode_file(gname.c_str(), uname.c_str()))
	{
	  return false;
	}
      file_to_diff = uname;
    }
  else
    {
      file_to_diff = gname;
    }

  /* When this function exits, delete the temporary file.
   */
  FileDeleter the_cleaner(file_to_diff);
  if (!flags.encoded)
    the_cleaner.disarm();


  seq_state sstate(highest_delta_seqno());
  const delta *got_delta = find_delta(pfile->got);
  if (got_delta == NULL)
    {
      errormsg("Locked delta doesn't exist!");
      return false;
    }

  // Remember seq number that will be the predecessor of the 
  // one for the delta.
  seq_no predecessor_seq = got_delta->seq;


  if (!prepare_seqstate(sstate, pfile->include, pfile->exclude,
		   sccs_date(NULL)))
    return false;
  
  if (!finalise_seqstate(sstate, predecessor_seq))
    return false;


  mystring dname(name.sub_file('d'));
  
//FILE *get_out = fopen(dname.c_str(), "wt");
  FILE *get_out = fcreate(dname, CREATE_EXCLUSIVE);
  if (NULL == get_out)
    {
      errormsg_with_errno("Cannot open file %s", dname.c_str());
      return false;
    }
  FileDeleter another_cleaner(dname);
  
  struct subst_parms parms(get_out, NULL, delta(), 
			   0, sccs_date(NULL));
  seq_state gsstate(sstate);

  get(dname, gsstate, parms, 0, 0, 0, 0, GET_NO_DECODE);

  if (fclose_failed(fclose(get_out)))
    {
      errormsg_with_errno("Failed to close temporary file");
      return false;
    }
  
	
  FileDiff differ(dname.c_str(), file_to_diff.c_str());
  FILE *diff_out = differ.start();

  
  // The delta operation consists of:-
  // 1. Writing out the information for the new delta.
  // 2. Writing out any automatic null deltas.
  // 3. Copying the body of the s-file to the output file,
  //    modifying it as indicated by the output of the diff
  //    program.
	
  if (false == seek_to_body())	// prepare to read the body for predecessor.
    return false;

  unsigned long int next_body_line = 1u;
  
  // Create any required null deltas if we need to.
  if (flags.null_deltas)
    {
      // figure out how many null deltas to make to fill the gap
      // between the highest current trunk release and the one
      // belonging to the new delta.

      // use our own comment.
      mylist<mystring> auto_comment;
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
  // assign a sequence number.
  seq_no new_seq = delta_table->next_seqno();

  
  // copy the list of excluded and included deltas from the p-file
  // into the delta.  pfile->include is a range_list<sid>,
  // but what we actually want to create is a list of seq_no.
  // The new delta header includes info about what deltas
  // are included, excluded, ignored.   Compute that now.
  mylist<seq_no> included;
  mylist<seq_no> excluded;

  
  if (!pfile->include.empty())
    {
      const_delta_iterator iter(delta_table);
      while (iter.next())
	{
	  if (pfile->include.member(iter->id))
	    {
	      included.add(iter->seq);
	    }
	}
    }
  if (!pfile->exclude.empty())
    {
      const_delta_iterator iter(delta_table);
      while (iter.next())
	{
	  if (pfile->exclude.member(iter->id))
	    {
	      excluded.add(iter->seq);
	    }
	}
    }
  
  // Construct the delta information for the new delta.
  delta new_delta('D', pfile->delta, sccs_date::now(),
		  get_user_name(), new_seq, predecessor_seq,
		  included, excluded, mrs, comments);

  // We don't know how many lines will be added/changed yet.
  // end_update() fixes that.
  new_delta.inserted = 0;
  new_delta.deleted = 0;
  new_delta.unchanged = 0;

	
  new_delta.id.print(stdout);
  printf("\n");
  
  // Begin the update by writing out the new delta.
  // This also writes out the information for all the
  // earlier deltas.
  FILE *out = start_update(new_delta);
  if (NULL == out)
    return false;
	
#undef DEBUG_FILE
#ifdef DEBUG_FILE
  FILE *df = fopen("delta.dbg", "w");
#endif

  // We have to continue while there is data on the input s-file,
  // or data from the diff, so we don't just stop when read_line()
  // returns -1.
  
  // int c = read_line(); // read line from old body.

  // xx begin JAY change Thu Apr 29 19:25:53 1999

  
  cssc_linebuf delta_buf;
  
  while (0 == delta_buf.read_line(diff_out)) // read_line returns 1 for EOF.
    {
      const char * pdiff = delta_buf.c_str();
      if (display_diff_output)
	(void)printf("%s", pdiff);
  
      // While we have a command in pdiff, interpret it.
      do
	{
	  char edit_type;		// a, d, c
	  int edit_line;		// location of next edit in old file
      
	  edit_line = diff_interp(pdiff, edit_type);
	  if (edit_line < 0)
	    {
	      return false;
	    }
	  
	  // if the edit type is APPEND, the next line from the diff
	  // output is to be appended.  Otherwise, the lines from the diff
	  // output will be inderted BEFORE the specified position (since
	  // left_line indicates the first line which is CHANGED or
	  // DELETED).
	  int target;
	  if ('a' == edit_type)
	    {
	      target = edit_line;
	    }
	  else 
	    {
	      target = edit_line - 1;
	    }

	  while (next_body_line <= target)
	    {
	      int increment = 0;
	      if (!traverse_body_line(sstate, out, false, &increment))
		{
		  return false; // I/O failure.
		}
	      
	      next_body_line += increment;
	      new_delta.unchanged += increment;
	    }
	  
	  switch (edit_type)
	    {
	    case 'a':
	      if (!delta_do_insert(out, delta_buf, new_delta,
				   new_seq, diff_out, display_diff_output))
		return false;
	      break;
	      
	    case 'd':
	      if (!delta_do_delete(out, delta_buf, new_delta,
				   new_seq, diff_out, display_diff_output,
				   sstate,
				   next_body_line))
		return false;
	      break;
	      
	    case 'c':
	      if (!delta_do_change(out, delta_buf, new_delta,
				   new_seq, diff_out, display_diff_output,
				   sstate,
				   next_body_line))
		return false;
	      break;
	      
	    default:
	      fatal_quit(-1, "(diff output): Unknown edit type: '%s'", pdiff);
	      break;
	    }
	} while (isdigit( (unsigned char) pdiff[0] ));
    }
  
  // reached EOF on delta output.  Any remaining lines must be unchanged.
  int increment = 0;
  while (traverse_body_line(sstate, out, true, &increment))
    {
      /* do nothing */
    }
  new_delta.unchanged += increment;
  if (ferror(out))
    {
      return false;
    }

#if 0
  if getenv("CSSC_DELTA_ABORT")
    {
      exit(13);
    }
#endif
  
  // xx end   JAY change Thu Apr 29 19:25:53 1999  

  
#ifdef DEBUG_FILE
  fclose(df);
#endif


  // The order of things that we do at this point is quite
  // important; we want only to update the s- and p- files if
  // everything worked.
  diff_out = 0;
	
  // It would be nice to know if the diff failed, but actually 
  // diff's return value indicates if there was a difference 
  // or not.
  differ.finish();

  pfile.delete_lock();
  
  end_update(out, new_delta);
      
  printf("%lu inserted\n%lu deleted\n%lu unchanged\n",
	 new_delta.inserted, new_delta.deleted, new_delta.unchanged);

  if (pfile.update())
    return true;
  else
    return false;
}

/* Local variables: */
/* mode: c++ */
/* End: */
