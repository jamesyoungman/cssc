/*
 * linebuf.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * Defines the class _linebuf.
 *
 * @(#) CSSC linebuf.h 1.1 93/11/09 17:17:47
 *
 */

#ifndef __LINEBUF_H__
#define __LINEBUF_H__

#ifdef __GNUC__
#pragma interface
#endif

/* This class is used to read lines of unlimited length from a file. */

class _linebuf {
	char *buf;
	int buflen;

public:
	_linebuf(): buf((char *) xmalloc(CONFIG_LINEBUF_CHUNK_SIZE)),
	           buflen(CONFIG_LINEBUF_CHUNK_SIZE) {}

	int read_line(FILE *f);

	operator char *() const { return buf; }
	char &operator [](int index) const { return buf[index]; }

#ifdef __GNUC__
	char *operator +(int index) const { return buf + index; }
#endif

	~_linebuf() { free(buf); }
};

#endif /* __LINEBUF_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
