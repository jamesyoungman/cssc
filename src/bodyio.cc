/*
 * bodyio.cc: Part of GNU CSSC.
 *
 *  Copyright (C) 1997, 1998, 1999, 2001, 2007, 2008, 2009, 2010, 2011,
 *  2014, 2019, 2024 Free Software Foundation, Inc.
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
 *
 * Code for performing I/O on the body of an SCCS file.
 * See also sf-admin.cc and encoding.cc.
 *
 */

#include <config.h>

#include <cstdio>
#include <cstring>

#include "cssc.h"
#include "bodyio.h"
#include "failure.h"
#include "quit.h"
#include "cssc-assert.h"
#include "filepos.h"
#include "except.h"
#include "linebuf.h"
#include "ioerr.h"
#include "file.h"

/* Check if we have exceeded the maximum line length.
 */
static bool check_line_len(const char *iname,
			   long int len_max,
			   int column,
			   int ch,
			   FILE *in)
{
  if (0 == len_max || column < len_max)
    {
      return true;
    }
  else
    {
      ungetc(ch, in);	// push back the character.

      if (binary_file_creation_allowed())
	{
	  errormsg("%s: line length exceeds %ld characters, "
		   "treating as binary.\n",
		   iname, len_max );
	}
      else
	{
	  errormsg("%s: line length exceeds %d characters, "
		   "and binary file support is disabled.\n",
		   iname, len_max );
	}
      return false;
    }
}




/* body_insert_text()
 *
 * Insert a file into an SCCS file (e.g. for admin).
 *
 * We fail if the input contains a ^A immediately following
 * a newline (which is special to SCCS), or if the input
 * does not end with a newline.
 *
 *  1) otherwise the control-character (^A) immediately
 *   following will not be recognised; SCCS utils
 *   only look for them at the beginning of a line
 *
 *  2) many diff(1) programs only cope with text files
 *   that end with a newline.
 */
cssc::Failure
body_insert_text(const char iname[], const char oname[],
		 FILE *in, FILE *out,
		 unsigned long int *lines,
		 bool *idkw)
{
  int ch, last;
  unsigned long int nl;		// number of lines.
  const long int len_max = max_sfile_line_len();
  bool found_id;
  int column;			// current column in ouutput file.

  // If we fail, rewind these files to try binary encoding.
  FilePosSaver o_saver(out);

  *idkw = found_id = false;
  nl = 0uL;
  last = '\n';

  column = 0;
  while ( EOF != (ch=getc(in)) )
    {
      if ('\n' == ch)
	++nl;

      // check for ^A at start of line.
      if ('\n' == last)
	{
	  column = 0;

	  if ('\001' == ch)
	    {
	      ungetc(ch, in);	// push back the control character.
	      // output file pointer implicitly rewound
	      return cssc::FailureBuilder(cssc::errorcode::ControlCharacterAtStartOfLine)
		<< iname << ": control character at start of line, "
		<< "treating as binary.\n";
	    }
	}
      else
	{
	  // TODO: are there too many increment operations here?
	  ++column;
	  if (!check_line_len(iname, len_max, ++column, ch, in))
	    {
	      // output file pointer implicitly rewound
	      return cssc::FailureBuilder(cssc::errorcode::BodyLineTooLong)
		<< iname << ": line is too long, treating as binary";
	    }
	}

      if (putc_failed(putc(ch, out)))
	{
	  return cssc::make_failure_builder_from_errno(errno)
	    .diagnose() << "write error on " << oname;
	}

      if (!found_id)		// Check for ID keywords.
	{
	  if ('%' == last && is_id_keyword_letter(static_cast<char>(ch)))
	    {
	      const int peek = getc(in);
	      if ('%' == peek)
		*idkw = found_id = true;
	      if (EOF != peek)	// Can't put the genie back in the bottle!
		ungetc(peek, in);
	    }
	}

      last = ch;
    }

  if (ferror(in))
    {
      return cssc::make_failure_builder_from_errno(errno)
	.diagnose() << "read error on " << iname;
    }


  // Make sure the file ended with a newline.
  if ('\n' != last)
    {
      // output file pointer implicitly rewound
      return cssc::FailureBuilder(cssc::errorcode::FileDoesNotEndWithNewline)
	<< iname << ": no newline at end of file, treating as binary";
    }

  // Success; do not rewind the output file.
  o_saver.disarm();
  *lines = nl;
  return cssc::Failure::Ok();
}

// Keywords: a file containing the octal characters
// 0026 0021 0141 (hex 0x16 0x11 0x61) will produce
// begin 0644 x
// #%A%A
// `
// end
//
//
// Stupidly imho, SCCS checks the ENCODED form of the file
// in order to support the "no ID keywords" warning.  Still,
// at least it doesn't expand those on output!


cssc::Failure
body_insert_binary(const char iname[], const char oname[],
		   FILE *in, FILE *out,
		   unsigned long int *lines,
		   bool *idkw)
{
  const int max_chunk = 45;
  char inbuf[max_chunk], outbuf[80];
  unsigned long int nl;
  size_t len;
  bool kw;
  *idkw = kw = false;

  nl = 0;
  while ( 0 < (len = fread(inbuf, sizeof(char), max_chunk, in)) )
    {
      encode_line(inbuf, outbuf, len); // see encoding.cc.

      if (!kw)
	{
	  // For some odd reason, SCCS seems to check
	  // the encoded form for ID keywords!  We know
	  // that strlen() on the UUENCODEd data is safe.
	  if (::check_id_keywords(outbuf, strlen(outbuf)))	// XXX used to check inbuf
	    *idkw = kw = true;
	}

      if (fputs_failed(fputs(outbuf, out)))
	{
	  return cssc::make_failure_builder_from_errno(errno)
	    .diagnose() << "write error on " << oname;
	}

      ++nl;
    }
  // A space character indicates a count of zero bytes and hence
  // the end of the encoded file.
  if (fputs_failed(fputs(" \n", out)))
    {
      return cssc::make_failure_builder_from_errno(errno)
	.diagnose() << "write error on " << oname;
    }

  ++nl;

  if (ferror(in))
    {
      return cssc::make_failure_builder_from_errno(errno)
	.diagnose() << "read error on " << iname;
    }

  *lines = nl;
  return cssc::Failure::Ok();
}


static cssc::Failure
copy_data(FILE *in, FILE *out)
{
  char buf[BUFSIZ];
  size_t n, nout;

  while ( 0u < (n=fread(buf, 1, BUFSIZ, in)))
    {
      nout = fwrite(buf, 1, n, out);
      if (nout < n)
	{
	  return cssc::make_failure_builder_from_errno(errno)
	    .diagnose() << "write error in copy_data";
	}
    }
  if (ferror(in))
    {
      return cssc::make_failure_builder_from_errno(errno)
	.diagnose() << "read error in copy_data";
    }
  else
    {
      return cssc::Failure::Ok();
    }
}


cssc::Failure
body_insert(bool *binary,
	    const char iname[], const char oname[],
	    FILE *in, FILE *out,
	    unsigned long int *lines,
	    bool *idkw)
{
  // If binary mode has not been forced, try text mode.
  if (*binary)
    {
      return body_insert_binary(iname, oname, in, out, lines, idkw);
    }
  else
    {
      // body_insert_text() takes care of rewinding the output
      // file; we may not even be able to rewind the input file.
      ASSERT(*binary == false);
      auto result = body_insert_text(iname, oname, in, out, lines, idkw);
      if (result.ok())
	return cssc::Failure::Ok();

      if (result.code() != cssc::make_error_condition(cssc::condition::BodyIsBinary))
	return result;

      *binary = true;
      if (!binary_file_creation_allowed())
	{
	  // We can't try again with a binary file, because that
	  // feature is disabled.
	  return cssc::FailureBuilder(result)
	    << "body is binary and encoding is not allowed";
	}

      // It wasn't text after all.  We may be reading from
      // stdin, so we can't seek on it.  But we have
      // the first segment of the file written to the x-file
      // already, and the remainder is still waiting to be
      // read, so we can recover all the data.
      FILE *tmp = tmpfile();
      if (!tmp)
	{
	  return cssc::make_failure_builder_from_errno(errno)
	    .diagnose() << "Could not create temporary file";
	}
      ResourceCleanup clean([tmp](){ fclose(tmp); });
      // Recover the data already written to the output
      // file and then rewind it, so that we can overwrite
      // it with the encoded version.
      FilePosSaver *fp_out = new FilePosSaver(out);
      cssc::Failure copy_done = copy_data(out, tmp);
      if (!copy_done.ok())
	{
	  return copy_done;
	}
      copy_done = copy_data(in,  tmp);
      if (!copy_done.ok())
	{
	  return copy_done;
	}
      delete fp_out;	// rewind the file OUT.
      rewind(tmp);
      return body_insert_binary("temporary file",
				oname, tmp, out, lines, idkw);
    }
}

cssc::Failure output_body_line_text(FILE *fp, const cssc_linebuf* plb)
{
  cssc::Failure result = plb->write(fp);
  if (!result.ok())
    return result;

  if (fputc_failed(fputc('\n', fp)))
    {
      return cssc::make_failure_from_errno(errno);
    }
    return cssc::Failure::Ok();
}

cssc::Failure output_body_line_binary(FILE *fp, const cssc_linebuf* plb)
{
  // Curiously, if the file is encoded, we know that
  // the encoded form is only about 60 characters
  // and contains no 8-bit or zero data.
  size_t n;
  char outbuf[80];

  n = decode_line(plb->c_str(), outbuf); // see encoding.cc
  return fwrite_failed(fwrite(outbuf, sizeof(char), n, fp), n);
}


cssc::Failure
encode_file(const char *nin, const char *nout)
{
  FILE *fin = fopen_as_real_user(nin, "rb");	// binary
  if (nullptr == fin)
    {
      return cssc::make_failure_builder_from_errno(errno)
	.diagnose() << "failed to open \"" << nin << "\" for reading";
    }
  ResourceCleanup cleanup_in([fin]() { fclose(fin); } );

  cssc::FailureOr<FILE*> fof = fcreate(nout, CREATE_EXCLUSIVE); // text
  if (!fof.ok())
    {
      return cssc::FailureBuilder(fof.fail())
	.diagnose() << "Failed to open " << nout << " for writing";
    }
  FILE *fout = *fof;

  ResourceCleanup cleanup_out([fout, nout]() {
      fclose(fout);
      remove(nout);
    });

  auto encoded = encode_stream(fin, fout);
  if (!encoded.first.ok())
    {
      return cssc::make_failure_builder(encoded.first)
	.diagnose() << "read error on " << nin;
    }
  if (!encoded.second.ok())
    {
      return cssc::make_failure_builder(encoded.second)
	.diagnose() << "write error on " << nout;
    }

  if (cleanup_in.disarm(), fclose_failed(fclose(fin)))
    {
      return cssc::make_failure_builder_from_errno(errno)
	.diagnose() << "failed to close " << nin;
    }

  if (fclose_failed(fclose(fout)))
    {
      return cssc::make_failure_builder_from_errno(errno)
	.diagnose() << "failed to close " << nout;
    }

  // Success, so don't delete the output file.
  cleanup_out.disarm();
  return cssc::Failure::Ok();
}
