/*
 * sf-write.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Members of the class sccs_file used update the SCCS file.
 *
 */
 
#include "cssc.h"
#include "sccsfile.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-write.cc,v 1.6 1997/05/31 10:22:02 james Exp $";
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
		if (fprintf(out, "\001%c", control) == EOF) {
			return 1;
		}
		for(i = 0; i < len; i++) {
			if (fprintf(out, " %u", seqs[i]) == EOF) {
				return 1;
			}
		}
		if (putc('\n', out) == EOF) {
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

	if (fprintf(out, "\001s %05u/%05u/%05u\n",
	            delta.inserted, delta.deleted, delta.unchanged) == EOF
	    || fprintf(out, "\001d %c ", delta.type) == EOF
	    || delta.id.print(out)
            || putc(' ', out) == EOF
            || delta.date.print(out)
            || fprintf(out, " %s %u %u\n", (const char *) delta.user,
		       delta.seq, delta.prev_seq) == EOF) {
		return 1;
	}

	if (print_seqs(out, 'i', delta.included)
	    || print_seqs(out, 'x', delta.excluded)
	    || print_seqs(out, 'g', delta.ignored)) {
		return 1;
	}

	len = delta.mrs.length();
	for(i = 0; i < len; i++) {
		if (fprintf(out, "\001m %s\n",
			    (const char *) delta.mrs[i]) == EOF) {
			return 1;
		}
	}

	len = delta.comments.length();
	for(i = 0; i < len; i++) {
		if (fprintf(out, "\001c %s\n",
			    (const char *) delta.comments[i]) == EOF) {
			return 1;
		}
	}

	return fputs("\001e\n", out) == EOF;
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

	if (fputs("\001u\n", out) == EOF) {
		return 1;
	}

	len = users.length();
	for(i = 0; i < len; i++) {
		s = users[i];
		assert(s[0] != '\001');
		if (fprintf(out, "%s\n", s) == EOF) {
			return 1;
		}
	}

	if (fputs("\001U\n", out) == EOF) {
		return 1;
	}

	s = flags.type;
	if (s != NULL && fprintf(out, "\001f t %s\n", s) == EOF) {
		return 1;
	}
       
	s = flags.mr_checker;
	if (s != NULL && fprintf(out, "\001f v %s\n", s) == EOF) {
		return 1;
	}

	if (flags.no_id_keywords_is_fatal) {
		if (fputs("\001f i\n", out) == EOF) {
			return 1;
		}
	}

	if (flags.branch && fputs("\001f b\n", out) == EOF) {
		return 1;
	}
		
	s = flags.module;
	if (s != NULL && fprintf(out, "\001f m %s\n", s) == EOF) {
		return 1;
	}

	if (flags.floor.valid()
	    && (fputs("\001f f ", out) == EOF
		|| flags.floor.print(out)
		|| putc('\n', out) == EOF)) {
		return 1;
	}

	if (flags.ceiling.valid()
	    && (fputs("\001f c ", out) == EOF
		|| flags.ceiling.print(out)
		|| putc('\n', out) == EOF)) {
		return 1;
	}

	if (flags.null_deltas && fputs("\001f n\n", out) == EOF) {
		return 1;
	}

	if (flags.joint_edit && fputs("\001f j\n", out) == EOF) {
		return 1;
	}

	if (flags.all_locked) {
		if (fputs("\001f l a\n", out) == EOF) {
			return 1;
		}
	} else if (!flags.locked.empty()
		   && (fputs("\001f l ", out) == EOF
		       || flags.locked.print(out)
		       || putc('\n', out) == EOF)) {
		return 1;
	}

	s = flags.user_def;
	if (s != NULL && fprintf(out, "\001f q %s\n", s) == EOF) {
		return 1;
	}
	
	/* Flag 'e': encoded flag -- boolean.
	 * Some versions of SCCS produce "\001 f a 0" if the file
	 * is not encoded, and some do not.  We shall not so as not
	 * to upset broken implementations, for example our own for the 
	 * time being.
	 */
	if (flags.encoded && fprintf(out, "\001f e 1") == EOF) {
                return 1;
	}
	
	s = flags.reserved;
	if (s != NULL && fprintf(out, "\001f z %s\n", s) == EOF) {
		return 1;
	}

	if (fputs("\001t\n", out) == EOF) {
		return 1;
	}

	len = comments.length();
	for(i = 0; i < len; i++) {
		s = comments[i];
		assert(s[0] != '\001');
		if (fprintf(out, "%s\n", s) == EOF) {
			return 1;
		}
	}

	if (fputs("\001T\n", out) == EOF) {
		return 1;
	}

	return 0;
}


/* End the update of the SCCS file by updating the checksum, and
   renaming the x-file to replace the old SCCS file. */

void
sccs_file::end_update(FILE *out) const {
#ifdef CONFIG_SYNC_BEFORE_REOPEN
	if (fflush(out) == EOF || ffsync(out) == EOF) {
#else
	if (fflush(out) == EOF) {
#endif

		xfile_error("Write error.");
	}
	rewind(out);

	unsigned sum;
	mystring xname = name.xfile();
	if (fclose(open_sccs_file(xname, READ, &sum)) == EOF) {
		xfile_error("Error closing file.");
	}

	if (fprintf(out, "\001h%05u", sum) == EOF
	    || fclose(out) == EOF) {
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

	if (fprintf(out, "\001h%05u", sum) == EOF
	    || fclose(out) == EOF) {
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
		if (fputs(linebuf, out) == EOF
		    || putc('\n', out) == EOF) {
			xfile_error("Write error.");
		}
	}

	end_update(out);
}

/* Local variables: */
/* mode: c++ */
/* End: */
