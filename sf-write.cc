/*
 * sf-write.c: Part of GNU CSSC.
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
 * Members of the class sccs_file used update the SCCS file.
 *
 */
 
#include "cssc.h"
#include "sccsfile.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-write.cc,v 1.8 1997/11/15 20:06:23 james Exp $";
#endif

/* Quit because an error related to the x-file. */

NORETURN
sccs_file::xfile_error(const char *msg) const {
	mystring xname = name.xfile();

	quit(errno, "%s: %s", (const char *) xname, msg);
}


/* Start the update of the SCCS file by creating the x-file which will
   become the new SCCS file and writing a dummy checksum line to it. */

FILE *
sccs_file::start_update() const {
	assert(mode != READ);

	if (mode == CREATE && file_exists(name)) {
		quit(-1, "%s: SCCS file already exists.",
		     (const char *) name);
	}
		     
	mystring xname = name.xfile();
	FILE *out = fcreate(xname, CREATE_READ_ONLY | CREATE_FOR_UPDATE);

	if (out == NULL) {
		xfile_error("Can't create temporary file for update.");
	}
	fputs("\001h-----\n", out);
	return out;
}


static int
print_seqs(FILE *out, char control, list<seq_no> const &seqs) {
	int i;
	int len = seqs.length();

	if (len != 0) {
		if (printf_failed(fprintf(out, "\001%c", control))) {
			return 1;
		}
		for(i = 0; i < len; i++) {
			if (printf_failed(fprintf(out, " %u", seqs[i]))) {
				return 1;
			}
		}
		if (putc_failed(putc('\n', out))) {
			return 1;
		}
	}
	return 0;
}

/* Outputs an entry to the delta table of a new SCCS file.
   Returns non-zero if an error occurs.  */

int
sccs_file::write_delta(FILE *out, struct delta const &delta) const {
	int len;
	int i;

	if (printf_failed(fprintf(out, "\001s %05u/%05u/%05u\n",
				  delta.inserted, delta.deleted,
				  delta.unchanged))
	    || printf_failed(fprintf(out, "\001d %c ", delta.type))
	    || delta.id.print(out)
            || putc_failed(putc(' ', out))
            || delta.date.print(out)
            || printf_failed(fprintf(out, " %s %u %u\n",
				     (const char *) delta.user,
				     delta.seq, delta.prev_seq))) {
		return 1;
	}

	if (print_seqs(out, 'i', delta.included)
	    || print_seqs(out, 'x', delta.excluded)
	    || print_seqs(out, 'g', delta.ignored)) {
		return 1;
	}

	len = delta.mrs.length();
	for(i = 0; i < len; i++) {
		if (printf_failed(fprintf(out, "\001m %s\n",
					  (const char *) delta.mrs[i]))) {
			return 1;
		}
	}

	len = delta.comments.length();
	for(i = 0; i < len; i++) {
		if (printf_failed(fprintf(out, "\001c %s\n",
					  (const char *) delta.comments[i]))) {
			return 1;
		}
	}

	return fputs_failed(fputs("\001e\n", out));
}


/* Writes everything up to the body to new SCCS file.  Returns non-zero
   if an error occurs. */

int
sccs_file::write(FILE *out) const {
	int len;
	int i;
	const char *s;

	delta_iterator iter(delta_table);
	while(iter.next(1)) {
#ifdef __GNUC__
		if (write_delta(out, *(iter.*&sccs_file::delta_iterator::operator->)())) {
#else
		if (write_delta(out, *iter.operator->())) {
#endif
			return 1;
		}
	}

	if (fputs_failed(fputs("\001u\n", out))) {
		return 1;
	}

	len = users.length();
	for(i = 0; i < len; i++) {
		s = users[i];
		assert(s[0] != '\001');
		if (printf_failed(fprintf(out, "%s\n", s))) {
			return 1;
		}
	}

	if (fputs_failed(fputs("\001U\n", out))) {
		return 1;
	}

	s = flags.type;
	if (s != NULL && printf_failed(fprintf(out, "\001f t %s\n", s))) {
		return 1;
	}
       
	s = flags.mr_checker;
	if (s != NULL && printf_failed(fprintf(out, "\001f v %s\n", s))) {
		return 1;
	}

	if (flags.no_id_keywords_is_fatal) {
		if (fputs_failed(fputs("\001f i\n", out))) {
			return 1;
		}
	}

	if (flags.branch && fputs_failed(fputs("\001f b\n", out))) {
		return 1;
	}
		
	s = flags.module;
	if (s != NULL && printf_failed(fprintf(out, "\001f m %s\n", s))) {
		return 1;
	}

	if (flags.floor.valid())
	  {
	    if (fputs_failed(fputs("\001f f ", out))
		|| flags.floor.print(out)
		|| putc_failed(putc('\n', out)))
	      {
		return 1;
	      }
	  }
	
	if (flags.ceiling.valid())
	  {
	    if(fputs_failed(fputs("\001f c ", out))
	       || flags.ceiling.print(out)
	       || putc_failed(putc('\n', out)))
	      {
		return 1;
	      }
	  }
	
	if (flags.null_deltas)
	  {
	    if (fputs_failed(fputs("\001f n\n", out)))
	      return 1;
	  }

	if (flags.joint_edit)
	  {
	    if (fputs_failed(fputs("\001f j\n", out)))
	      return 1;
	  }

	if (flags.all_locked) {
		if (fputs_failed(fputs("\001f l a\n", out))) {
		  return 1;
		}
	} else if (!flags.locked.empty()
		   && (fputs_failed(fputs("\001f l ", out))
		       || flags.locked.print(out)
		       || putc_failed(putc('\n', out)))) {
		return 1;
	}

	s = flags.user_def;
	if (s != NULL && printf_failed(fprintf(out, "\001f q %s\n", s))) {
		return 1;
	}
	
	/* Flag 'e': encoded flag -- boolean.
	 * Some versions of SCCS produce "\001 f a 0" if the file
	 * is not encoded, and some do not.  We shall not so as not
	 * to upset broken implementations, for example our own for the 
	 * time being.
	 */
	if (flags.encoded)
	  {
	    if (printf_failed(fprintf(out, "\001f e 1")))
	      return 1;
	  }
	
	s = flags.reserved;
	if (s != NULL && printf_failed(fprintf(out, "\001f z %s\n", s))) {
		return 1;
	}

	if (fputs_failed(fputs("\001t\n", out)))
	  {
	    return 1;
	  }

	len = comments.length();
	for(i = 0; i < len; i++) {
		s = comments[i];
		assert(s[0] != '\001');
		if (printf_failed(fprintf(out, "%s\n", s))) {
			return 1;
		}
	}

	if (fputs_failed(fputs("\001T\n", out)))
	  {
	    return 1;
	  }

	return 0;
}


/* End the update of the SCCS file by updating the checksum, and
   renaming the x-file to replace the old SCCS file. */

void
sccs_file::end_update(FILE *out) const {
#ifdef CONFIG_SYNC_BEFORE_REOPEN
	if (fflush_failed(fflush(out)) || ffsync(out) == EOF) {
#else
	if (fflush_failed(fflush(out))) {
#endif

		xfile_error("Write error.");
	}
	rewind(out);

	unsigned sum;
	mystring xname = name.xfile();
	if (fclose_failed(fclose(open_sccs_file(xname, READ, &sum)))) {
		xfile_error("Error closing file.");
	}

	if (printf_failed(fprintf(out, "\001h%05u", sum))
	    || fclose_failed(fclose(out)))
	  {
	    xfile_error("Write error.");
	  }

#ifndef TESTING	

	if (mode != CREATE && remove(name) == -1) {
		quit(errno, "%s: Can't remove old SCCS file.",
		     (const char *) name);
	}

	if (rename(xname, name) == -1) {
		xfile_error("Can't rename new SCCS file.");
	}

#endif
}



/* Recalculate and update the checksum of a SCCS file. */

void
sccs_file::update_checksum(const char *name) {
	unsigned sum;
	FILE *out = open_sccs_file(name, UPDATE, &sum);

	if (fprintf_failed(fprintf(out, "\001h%05u", sum))
	    || fclose_failed(fclose(out)))
	  {
	    quit(errno, "%s: Write error.",
		 (const char *) name);
	  }
}


/* Update the SCCS file */

void
sccs_file::update() {
	assert(mode != CREATE);

	FILE *out = start_update();
	if (write(out)) {
		xfile_error("Write error.");
	}

	seek_to_body();
	while(read_line() != -1) {
		if (fputs_failed(fputs(linebuf, out))
		    || putc_failed(putc('\n', out))) {
			xfile_error("Write error.");
		}
	}

	end_update(out);
}

/* Local variables: */
/* mode: c++ */
/* End: */
