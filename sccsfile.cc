/*
 * sccsfile.c 
 *
 * By Ross Ridge
 * Public Domain
 *
 * Common members of the class sccs_file and its subclasses.  Most of
 * the members in this file are used to read from the SCCS file.
 *
 */

#include "mysc.h"
#include "sccsfile.h"

#include <ctype.h>

#ifdef CONFIG_SCCS_IDS
static const char sccs_id[] = "@(#) MySC sccsfile.c 1.2 93/11/13 05:48:45";
#endif

/* struct */ sccs_file::delta &
sccs_file::delta::operator =(struct delta const &it) {
	inserted = it.inserted;
	deleted = it.deleted;
	unchanged = it.unchanged;
	type = it.type;
	id = it.id;
	date = it.date;
	user = it.user;
	seq = it.seq;
	prev_seq = it.prev_seq;
	included = it.included;
	excluded = it.excluded;
	ignored = it.ignored;
	mrs = it.mrs;
	comments = it.comments;
	return *this;
}


/* Builds the seq_no to delta table index table. */

void
sccs_file::_delta_table::build_seq_table() {
	seq_table = (int *) xmalloc((high_seqno + 1) * sizeof(int));

	int i;
	for(i = 0; i < high_seqno + 1; i++) {
		seq_table[i] = -1;
	}

	delta_iterator iter(*this);
	while(iter.next(1)) {
		seq_no seq = iter->seq;
		if (seq_table[seq] != -1) {
			quit(-1, "Sequence number %u is duplicated"
			         " in delta table.", seq);
		}
		seq_table[seq] = iter.index();
	}
}


void
sccs_file::_delta_table::update_highest(struct delta const &it) {
	seq_no seq = it.seq;
	if (seq > high_seqno) {
		if (seq_table != NULL) {
			seq_table = (int *) xrealloc(seq_table, 
						     (seq + 1) * sizeof(int));
			int i;
			for(i = high_seqno + 1; i < seq + 1; i++) {
				seq_table[i] = -1;
			}
		}
		high_seqno = it.seq;
	}

	if (it.id > high_release) {
		high_release = it.id;
	}

	if (seq_table != NULL) {
		if (seq_table[seq] != -1) {
			quit(-1, "Sequence number %u is duplicated"
			         " in delta table.", seq);
		}
		seq_table[seq] = length() - 1;
	}
}


/* Adds a delta to the end of the delta_table. */

void
sccs_file::_delta_table::add(struct delta const &it) {
	list<struct delta>::add(it);
	update_highest(it);
}


/* Finds a delta in the delta table by its SID. */

/* struct */ sccs_file::delta const *
sccs_file::_delta_table::find(sid id) const {
	delta_iterator iter(*this);

	while(iter.next()) {
		if (iter->id == id) {
#ifdef __GNUC__
			return (iter.*&sccs_file::delta_iterator::operator->)();
#else
			return iter.operator->();
#endif
		}
	}

	return NULL;
}


/* Static member for opening a SCCS file and then calculating its checksum. */

FILE *
sccs_file::open_sccs_file(const char *name, enum _mode mode, unsigned *sump) {
	FILE *f;

#ifdef CONFIG_BINARY_FILE
	f = fopen(name, "rb");
#else
	if (mode == UPDATE) {
		f = fopen(name, "r+");
	} else {
		f = fopen(name, "r");
	}
#endif

	if (f == NULL) {
		quit(errno, "%s: Can't open SCCS file for reading.", name);
	}

	if (getc(f) != '\001' || getc(f) != 'h') {
		quit(-1, "%s: Bad magic number", name);
	}

	int c = getc(f);
	while(c != CONFIG_EOL_CHARACTER) {
		if (c == EOF) {
			quit(errno, "%s: Unexpected EOF.", name);
		}
		c = getc(f);
	}

	unsigned sum = 0;
	c = getc(f);
	while(c != EOF) {
		sum += (unsigned char) c;
		c = getc(f);
	}

	if (ferror(f)) {
		quit(errno, "%s: Read error.", (const char *) name);
	}

	*sump = sum & 0xFFFF;

#ifdef CONFIG_BINARY_FILE
	fclose(f);
	if (mode == UPDATE) {
		f = fopen(name, "r+");
	} else {
		f = fopen(name, "r");
	}
	if (f == NULL) {
		quit(errno, "%s: Can't open SCCS file for reading.", name);
	}
#else
	rewind(f);
#endif

	return f;
}


/* Reads a line from the SCCS file and increments the current line number.
   If the end of file is reached it returns -1.  If it's a control line
   (it starts with ^A) then the control character (the second character)
   is returned.  Otherwise 0 is returned. */

int
sccs_file::read_line() {
	if (read_line_param(f)) {
		if (ferror(f)) {
			quit(errno, "%s: Read error.", (const char *) name);
		}
		return -1;
	} 

	lineno++;
	if (linebuf[0] == '\001') {
#if 0
		fprintf(stderr, "@%s\n", (const char *) linebuf + 1);
#endif
		return linebuf[1];
	}
#if 0
	fprintf(stderr, ":%s\n", (const char *) linebuf);
#endif
	return 0;

}


/* Quits with a message saying that SCCS file is corrupt. */

NORETURN
sccs_file::corrupt(const char *why) const {
	quit(-1, "%s: line %d: Corrupted SCCS file. (%s)",
	     (const char *) name, lineno, why);
}


/* Checks that a control line has at least one argument. */

void
sccs_file::check_arg() const {
	if (linebuf[2] != ' ') {
		corrupt("Missing arg");
	}
}


/* Checks the a control line has no arguments. */

void
sccs_file::check_noarg() const {
	if (linebuf[2] != '\0') {
		corrupt("Unexpected arg");
	}
}


/* Converts an ASCII string to an unsigned short, quiting if the
   string isn't a valid number. */

unsigned short
sccs_file::strict_atous(const char *s) const {
	long n = 0;

	char c = *s++;
	while(c != '\0') {
		if (!isdigit(c)) {
			corrupt("Invalid number");
		}
		n = n * 10 + (c - '0');
		if (n > 65535L) {
			corrupt("Number too big");
		}
		c = *s++;
	}

	return (unsigned short) n;
}

/* Reads a delta from the SCCS file's delta table and adds it to the
   delta table. */

void
sccs_file::read_delta() {

	/* The current line should be an 's' control line */

	assert(linebuf[1] == 's');
	check_arg();
	
	char *args[7];		/* Stores the result of spliting a line */

	if (split(linebuf + 3, args, 3, '/') != 3) {
		corrupt("Two /'s expected");
	}

	struct delta tmp;	/* The new delta */

	tmp.inserted = strict_atous(args[0]);
	tmp.deleted = strict_atous(args[1]);
	tmp.unchanged = strict_atous(args[2]);

	if (read_line() != 'd') {
		corrupt("Expected '@d'");
	}

	check_arg();

	split(linebuf + 3, args, 7, ' ');

	tmp.type = args[0][0];
	tmp.id = sid(args[1]);
	tmp.date = sccs_date(args[2], args[3]);
	tmp.user = args[4];
	tmp.seq = strict_atous(args[5]);
	tmp.prev_seq = strict_atous(args[6]);

	if ((tmp.type != 'R' && tmp.type != 'D') || args[0][1] != '\0') {
		corrupt("Bad delta type");
	}
	if (!tmp.id.valid()) {
		corrupt("Bad SID");
	}
	if (!tmp.date.valid()) {
		corrupt("Bad Date/Time");
	}

	/* Read in any lists of included, excluded or ignored seq. no's. */

	int c = read_line();
	int i;
 	const char *start;
	for(i = 0; i < 3; i++) {
		if (c == "ixg"[i]) {
			check_arg();

			start = linebuf + 3;
			do {
				char *end = strchr(start, ' ');
				if (end != NULL) {
					*end++ = '\0';
				}
				seq_no seq = strict_atous(start);
				switch(c) {
				case 'i':
					tmp.included.add(seq);
					break;

				case 'x':
					tmp.excluded.add(seq);
					break;

				case 'g':
					tmp.ignored.add(seq);
					break;
				}
				start = end;
			} while(start != NULL);

			c = read_line();
		}
	}

	while(c == 'm') {
		check_arg();
		tmp.mrs.add(linebuf + 3);
		c = read_line();
	}

	while(c == 'c') {
		check_arg();
		tmp.comments.add(linebuf + 3);
		c = read_line();
	}

	if (c != 'e') {
		corrupt("Expected '@e'");
	}

	check_noarg();

	delta_table.add(tmp);
}


/* Seeks on the SCCS file to the start of the body.  This function 
   may be rewritten as fseek() doesn't always work too well on
   text files. */

void
sccs_file::seek_to_body() {
	if (fseek(f, body_offset, 0) != 0) {
		quit(errno, "%s: fseek() failed!", (const char *) name);
	}
	lineno = body_lineno;
}



/* Returns the module name of the SCCS file. */

mystring
sccs_file::get_module_name() const {
	if (flags.module == NULL) {
		return name.gfile();
	}
	return flags.module;
}

/* Constructor for the class sccs_file.  Unless the SCCS file is being
   created it reads in the all but the body of the file.  The file is
   locked if it isn't only being read.  */

sccs_file::sccs_file(sccs_name &n, enum _mode m)
	: name(n), mode(m), lineno(0) {

	if (!name.valid()) {
		quit(-1, "%s: Not an SCCS file.", (const char *) name);
	}

	flags.branch = 0;
	flags.floor = NULL;
	flags.ceiling = NULL;
	flags.default_sid = NULL;
	flags.null_deltas = 0;
	flags.joint_edit = 0;
	flags.all_locked = 0;
	flags.encoded = 0;
	
	if (mode != READ) {
		if (name.lock()) {
			quit(-1, "%s: SCCS file is locked.  Try again later.",
			     (const char *) name);
		}
	}

	if (mode == CREATE) {
		return;
	}

	unsigned sum;
	f = open_sccs_file(name, READ, &sum);

	int c = read_line();
	assert(c == 'h');

	if (strict_atous(linebuf + 2) != sum) {
		fprintf(stderr, "%s: Warning bad checksum.\n",
			(const char *) name);
	}
	
	c = read_line();
	while(c == 's') {
		read_delta();
		c = read_line();
	}

	if (c != 'u') {
		corrupt("Expected '@u'");
	}

	check_noarg();

	c = read_line();
	while(c != 'U') {
		if (c != 0) {
			corrupt("User name expected.");
		}
		users.add((const char *)linebuf);
		c = read_line();
	}

	check_noarg();

	c = read_line();
	while(c == 'f') {
		check_arg();

		if (linebuf[3] == '\0'
		    || (linebuf[4] != '\0' && linebuf[4] != ' ')) {
			corrupt("Bad flag arg.");
		}

		char *arg = NULL;
		if (linebuf[4] == ' ') {
			arg = linebuf + 5;
		}

		switch(linebuf[3]) {
		case 't':
			flags.type = arg;
			break;

		case 'v':
			flags.mr_checker = arg;
			break;

		case 'i':
			if (arg == NULL) {
				flags.id_keywords = "";
			} else {
				flags.id_keywords = arg;
			}
			break;

		case 'b':
			flags.branch = 1;
			break;

		case 'm':
			flags.module = arg;
			break;

		case 'f':
			flags.floor = release(arg);
			if (!flags.floor.valid()) {
				corrupt("Bad 'f' flag");
			}
			break;

		case 'c':
			flags.ceiling = release(arg);
			if (!flags.ceiling.valid()) {
				corrupt("Bad 'c' flag");
			}
			break;

		case 'd':
			flags.default_sid = sid(arg);
			if (!flags.default_sid.valid()) {
				corrupt("Bad 'd' flag");
			}
			break;

		case 'n':
			flags.null_deltas = 1;
			break;

		case 'j':
			flags.joint_edit = 1;
			break;

		case 'l':
			if (arg != NULL && strcmp(arg, "a") == 0) {
				flags.all_locked = 1;
			} else {
				flags.locked = release_list(arg);
			}
			break;

		case 'q':
			flags.user_def = arg;
			break;

		case 'z':
			flags.reserved = arg;
			break;
		case 'e':
			if ('1' == *arg)
		          {
			    fprintf(stderr,
				    "%s: Warning: encoded files not "
				    "fully supported.\n", (const char *) name);
			    flags.encoded = 1;
			  }
		        else if ('0' == *arg)
			  {
			    flags.encoded = 0;
			  }
		        else
			  {
			    corrupt("Bad value for 'e' flag.");
			  }
			break;

		default:
			corrupt("Unknown flag.");
		}

		c = read_line();
	}

	if (c != 't') {
		corrupt("Expected '@t'");
	}

	check_noarg();

	c = read_line();
	while(c == 0) {
		comments.add((const char *)linebuf);
		c = read_line();
	}

	if (c != 'T') {
		corrupt("Expected '@T'");
	}

	check_noarg();

	body_offset = ftell(f);
	if (body_offset == -1L) {
		quit(errno, "ftell() failed.");
	}

	body_lineno = lineno;
}


/* Find the SID of the most recently created delta with the same release
   and level as the requested SID. */
   
sid
sccs_file::find_most_recent_sid(sid id) const {
	sccs_date newest;
	sid found;
	delta_iterator iter(delta_table);

#if 0
	fputs("find_most_recented_sid(", stderr);
	id.dprint(stderr);
	fputs(")\n", stderr);
#endif

	while(iter.next()) {
		if (id.trunk_match(iter->id)) {
#if 0
			fputs("match: ", stderr);
			iter->id.dprint(stderr);
			putc('\n', stderr);
#endif
			if (found.is_null() || newest < iter->date) {
				newest = iter->date;
				found = iter->id;
			}
		}
	}
	return found;
}


/* Destructor for class sccs_file. */

sccs_file::~sccs_file() {
	if (mode != READ) {
		name.unlock();
	}
	if (mode != CREATE) {
		fclose(f);
	}
}

/* Local variables: */
/* mode: c++ */
/* End: */
