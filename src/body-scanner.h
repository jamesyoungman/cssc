/*
 * body-scanner.h: Part of GNU CSSC.
 *
 *  Copyright (C) 2019 Free Software Foundation, Inc.
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
 */
#ifndef CSSC__BODY_SCANNER_H__
#define CSSC__BODY_SCANNER_H__

#include <cstdio>
#include <sys/types.h>		/* off_t */
#include <string>
#include <functional>
#include <system_error>

#include "base-reader.h"
#include "delta.h"		/* for seq_no */
#include "failure.h"
#include "location.h"

class cssc_linebuf;
class cssc_delta_table;
class seq_state;

struct delta_result
{
  delta_result() : success(false), inserted(0), deleted(0), unchanged(0) {}

  bool success;
  unsigned long inserted;
  unsigned long deleted;
  unsigned long unchanged;
};

class sccs_file_body_scanner : public sccs_file_reader_base
{
public:
  // sccs_file_body_scanner takes ownership of f.
  sccs_file_body_scanner(const std::string& filename, FILE*f, off_t body_pos, long body_pos_line_number);
  ~sccs_file_body_scanner();

  cssc::Failure get(const std::string& gname, const cssc_delta_table&,
		    std::function<cssc::Failure(const char *start,
						struct delta const& gotten_delta,
						bool force_expansion)> write_subst,
		    cssc::Failure (*outputfn)(FILE*,const cssc_linebuf*),
		    bool encoded,
		    class seq_state &state, struct subst_parms &parms,
		    bool do_kw_subst, bool debug, bool show_module, bool show_sid);
  delta_result
  delta(const std::string& dname, const std::string& file_to_diff,
	seq_no highest_delta_seqno, seq_no new_seq_no, seq_state*, FILE* out,
	bool display_diff_output);

  cssc::Failure seek_to_body();
  bool copy_to(FILE*);
  bool emit_raw_body(FILE*, const char*);
  bool remove(FILE*, seq_no id);

  // Print the body of an SCCS file to |out|, transforming all "^A"s
  // into "*** "s.  The name of the output file is |name|.
  bool print_body(FILE* out, const std::string& name);

private:
  FILE* f_;
  // TODO: rationalise the body_start_ / start_ overcomplexity
  off_t body_start_;
  sccs_file_location start_;
};


#endif /* CSSC__BODY_SCANNER_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
