/*
 * sf-write.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 1997, 1998, 1999, 2001, 2003, 2004, 2007, 2008, 2009,
 *  2010, 2011, 2014, 2019, 2024 Free Software Foundation, Inc.
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
 *
 *
 * Members of the class sccs_file used update the SCCS file.
 *
 */

#include <config.h>
#include <string>

#include "cssc.h"
#include "failure.h"
#include "sccsfile.h"
#include "delta.h"
#include "delta-iterator.h"
#include "delta-table.h"
#include "ioerr.h"
#include "linebuf.h"
#include "failure.h"
#include "filepos.h"
#include "file.h"
#include "ioerr.h"

using cssc::Failure;

/* Quit because an error related to the x-file. */
void
sccs_file::xfile_error(const char *msg) const
{
  errormsg_with_errno("%s: %s", name_.xfile().c_str(), msg);
}


/* Start the update of the SCCS file by creating the x-file which will
   become the new SCCS file and writing a dummy checksum line to it. */

cssc::FailureOr<FILE*>
sccs_file::start_update() {
        ASSERT(mode_ != READ);

        if (mode_ == CREATE && file_exists(name_.c_str()))
          {
	    return cssc::make_failure_builder(cssc::errorcode::DeclineToCreateHistoryFileThatAlreadyExists)
	      .diagnose() << name_.sfile() << ": SCCS file already exists";
          }

        std::string xname = name_.xfile();

        // real SCCS silently destroys any existing file named
        // x.[whatever].  Dave Bodenstab <imdave@mcs.net> pointed out
        // that this isn't very friendly.  However, I don't want to
        // abort if x.foo exists, since "real" SCCS doesn't.  So we'll
        // produce a warning and rename the old file.  If x.foo.bak exists,
        // we fail to rename, and lose the original x.foo file, as does
        // the genuine article.
        if (file_exists(xname.c_str()))
          {
            const char *xns = xname.c_str();
            std::string newname(xname + ".bak");
            if (0 == rename(xns, newname.c_str()))
              {
                warning("%s renamed to %s\n",
			xns, newname.c_str());
              }
            else
              {
                warning("%s over-written!\n", xns);
              }
          }


	// The 'x' flag is a SCO extension.
	const int x = sfile_should_be_executable() ? CREATE_EXECUTABLE : 0;
	cssc::FailureOr<FILE *> fof = fcreate(xname, CREATE_READ_ONLY | CREATE_FOR_UPDATE | x);
        if (!fof.ok())
          {
	    return cssc::make_failure_builder(fof.fail())
	      .diagnose() <<  xname << ": can't create temporary file for update";
          }
        else
          {
	    FILE *out = *fof;
            xfile_created_ = true;

            if (fputs_failed(fputs("\001h-----\n", out)))
              {
		const int saved_errno = errno;
                fclose(out);
		return cssc::make_failure_builder_from_errno(saved_errno)
		  .diagnose() << "failed to write to " << xname;
              }
            return out;
          }
}


static cssc::Failure
print_seqs(FILE *out, char control, std::vector<seq_no> const &seqs) {
  if (!seqs.empty())
    {
      if (printf_failed(fprintf(out, "\001%c", control)))
	return cssc::make_failure_from_errno(errno);

      for (const auto& seq : seqs)
	{
	  if (printf_failed(fprintf(out, " %u", seq))) {
	    return cssc::make_failure_from_errno(errno);
	  }
	}
      if (putc_failed(putc('\n', out)))
	return cssc::make_failure_from_errno(errno);
    }
  return cssc::Failure::Ok();
}

/* Outputs an entry to the delta table of a new SCCS file.
   Returns non-zero if an error occurs.  */

cssc::Failure
sccs_file::write_delta(FILE *out, struct delta const &d) const
{
  if (printf_failed(fprintf(out, "\001s %05lu/%05lu/%05lu\n",
                            cap5(d.inserted()),
                            cap5(d.deleted()),
                            cap5(d.unchanged())))
      || printf_failed(fprintf(out, "\001d %c ", d.get_type())))
    {
      return cssc::make_failure_from_errno(errno);
    }
  cssc::Failure done = d.id().print(out);
  if (!done.ok())
      return done;

  if (putc_failed(putc(' ', out)))
      return cssc::make_failure_from_errno(errno);
  done = d.date().print(out);
  if (!done.ok())
    return done;

  if (printf_failed(fprintf(out, " %s %u %u\n",
			    d.user().c_str(),
			    d.seq(), d.prev_seq())))
    {
      return cssc::make_failure_from_errno(errno);
    }
  done = cssc::Update(done, print_seqs(out, 'i', d.get_included_seqnos()));
  done = cssc::Update(done, print_seqs(out, 'x', d.get_excluded_seqnos()));
  done = cssc::Update(done, print_seqs(out, 'g', d.get_ignored_seqnos()));
  if (!done.ok())
    return done;

  for (const auto& mr : d.mrs())
    {
      if (printf_failed(fprintf(out, "\001m %s\n", mr.c_str())))
	return cssc::make_failure_from_errno(errno);
    }

  for (const auto& comment : d.comments())
    {
      if (printf_failed(fprintf(out, "\001c %s\n", comment.c_str())))
	return cssc::make_failure_from_errno(errno);
    }

  if (fputs_failed(fputs("\001e\n", out)))
    return cssc::make_failure_from_errno(errno);
  return cssc::Failure::Ok();
}


/* Writes everything up to the body to new SCCS file.  Returns non-zero
   if an error occurs. */

cssc::Failure
sccs_file::write(FILE *out) const
{
  cssc::Failure result = [this, out]()
    {
      const_delta_iterator iter(delta_table_.get(), delta_selector::all);
      while (iter.next())
	{
	  cssc::Failure written = write_delta(out, *iter.operator->());
	  if (!written.ok())
	    return written;
	}

      if (fputs_failed(fputs("\001u\n", out)))
	{
	  return cssc::make_failure_from_errno(errno);
	}

      for (const auto& user : users_)
	{
	  ASSERT(user[0] != '\001');
	  if (printf_failed(fprintf(out, "%s\n", user.c_str())))
	    return cssc::make_failure_from_errno(errno);
	}

      if (fputs_failed(fputs("\001U\n", out))) {
	return cssc::make_failure_from_errno(errno);
      }

      // Beginning of flags...

      // b branch
      if (flags.branch && fputs_failed(fputs("\001f b\n", out)))
	{
	  return cssc::make_failure_from_errno(errno);
	}

      // c ceiling
      if (flags.ceiling.valid())
	{
	  if (fputs_failed(fputs("\001f c ", out)))
	    return cssc::make_failure_from_errno(errno);

	  auto written = flags.ceiling.print(out);
	  if (!written.ok())
	    return written;

	  if (putc_failed(putc('\n', out)))
	    return cssc::make_failure_from_errno(errno);
	}

      // d default SID
      if (flags.default_sid.valid())
	{
	  if (fputs_failed(fputs("\001f d ", out)))
	    return cssc::make_failure_from_errno(errno);
	  auto written = flags.default_sid.print(out);
	  if (!written.ok())
	    return written;
	  if (putc_failed(putc('\n', out)))
	    return cssc::make_failure_from_errno(errno);
	}

      // f floor
      if (flags.floor.valid())
	{
	  if (fputs_failed(fputs("\001f f ", out)))
	    return cssc::make_failure_from_errno(errno);

	  auto written = flags.floor.print(out);
	  if (!written.ok())
	    return written;

	  if (putc_failed(putc('\n', out)))
	    return cssc::make_failure_from_errno(errno);
	}

      // i no id kw is error
      if (flags.no_id_keywords_is_fatal)
	{
	  if (fputs_failed(fputs("\001f i\n", out)))
	    {
	      return cssc::make_failure_from_errno(errno);
	    }
	}

      // j joint-edit
      if (flags.joint_edit)
	{
	  if (fputs_failed(fputs("\001f j\n", out)))
	    return cssc::make_failure_from_errno(errno);
	}

      // l locked-releases
      if (flags.all_locked)
	{
	  if (fputs_failed(fputs("\001f l a\n", out)))
	    {
	      return cssc::make_failure_from_errno(errno);
	    }
	}
      else if ( !flags.locked.empty() )
	{
	  if (fputs_failed(fputs("\001f l ", out)))
	    return cssc::make_failure_from_errno(errno);

	  if (!flags.locked.print(out).ok())
	    return cssc::make_failure_from_errno(errno);

	  if (putc_failed(putc('\n', out)))
	    return cssc::make_failure_from_errno(errno);
	}

      // m Module flag
      if (flags.module)
	{
	  const char *p = flags.module->c_str();
	  if (printf_failed(fprintf(out, "\001f m %s\n", p)))
	    {
	      return cssc::make_failure_from_errno(errno);
	    }
	}

      // n Create empty deltas
      if (flags.null_deltas)
	{
	  if (fputs_failed(fputs("\001f n\n", out)))
	    return cssc::make_failure_from_errno(errno);
	}

      // q %Q% subst value (user flag)
      if (flags.user_def)
	{
	  const char *p = flags.user_def->c_str();
	  if (printf_failed(fprintf(out, "\001f q %s\n", p)))
	    {
	      return cssc::make_failure_from_errno(errno);
	    }
	}

      // t %Y% subst value
      if (flags.type)
	{
	  if (printf_failed(fprintf(out, "\001f t %s\n",
				    flags.type->c_str())))
	    {
	      return cssc::make_failure_from_errno(errno);
	    }
	}

      // v MR-validation program.
      if (flags.mr_checker)
	{
	  if (printf_failed(fprintf(out, "\001f v %s\n",
				    flags.mr_checker->c_str())))
	    return cssc::make_failure_from_errno(errno);
	}

      // Write the correct value for the "encoded" flag.
      // We have to write it even if the flag is unset,
      // because "admin -i" goes back and updates that byte if the file
      // turns out to have been binary.
      if (printf_failed(fprintf(out, "\001f e %c\n",
				(flags.encoded ? '1' : '0'))))
	{
	  return cssc::make_failure_from_errno(errno);
	}


      // x - executable flag (a SCO extension)
      // Setting execute perms on the history file is a more portable way to
      // achieve what the user probably wants.
      if (flags.executable)
	{
	  if (printf_failed(fprintf(out, "\001f x\n")))
	    {
	      return cssc::make_failure_from_errno(errno);
	    }
	}

      // y - substituted keywords (a Sun Solaris 8+ extension)
      if (!flags.substitued_flag_letters.empty())
	{
	  if (printf_failed(fprintf(out, "\001f y ")))
	    return cssc::make_failure_from_errno(errno);

	  cssc::Failure printed = print_subsituted_flags_list(out, " ");
	  if (!printed.ok())
	    return printed;

	  if (printf_failed(fprintf(out, "\n")))
	    return cssc::make_failure_from_errno(errno);
	}

      if (flags.reserved)
	{
	  if (printf_failed(fprintf(out, "\001f z %s\n",
				    flags.reserved->c_str())))
	    {
	      return cssc::make_failure_from_errno(errno);
	    }
	}

      // end of flags.

      if (fputs_failed(fputs("\001t\n", out)))
	{
	  return cssc::make_failure_from_errno(errno);
	}

      for (const auto& comment : comments_)
	{
	  ASSERT(comment[0] != '\001');
	  if (printf_failed(fprintf(out, "%s\n", comment.c_str())))
	    {
	      return cssc::make_failure_from_errno(errno);
	    }
	}

      if (fputs_failed(fputs("\001T\n", out)))
	{
	  return cssc::make_failure_from_errno(errno);
	}

      return cssc::Failure::Ok();
    }();
  if (!result.ok())
    {
      return (cssc::FailureBuilder(result)
	      .diagnose() << "failed to write file header");
    }
  return cssc::Failure::Ok();
}

Failure
sccs_file::rehack_encoded_flag(FILE *fp, int *sum) const
{
  // Find the encoded flag.  Maybe change it.
  // "f" must be opened for update.
  int ch, last;
  last = '\n';
  const char match[] = "\001f e ";
  const int nmatch = strlen(match);
  int n;

  errno = 0;
  while (EOF != (ch=getc(fp)))
    {
      if ('\n' == last)
        {
          n = 0;
          while (match[n] == ch)
            {
              if (nmatch-1 == n)
                {
                  // success; now we might change the flag.
                  FilePosSaver *pos = new FilePosSaver(fp);
                  ch = getc(fp);
                  if ('0' == ch)
                    {
                      delete pos; // rewind file 1 char.
                      putc('1', fp);
                      const int d =  ('1' - '0'); // change to checksum.
                      *sum = (*sum + d) & 0xFFFF; // adjust checksum.
                      return cssc::Failure::Ok();
                    }
                  else
                    {
		      const int saved_errno = errno;
                      pos->disarm();
                      delete pos;
		      if (!saved_errno)
			return cssc::Failure::Ok();// flag was already set.
		      return cssc::make_failure_from_errno(errno);
                    }
                }
              ++n;              // advance match.
              ch = getc(fp);
	      if (EOF == ch)
		{
		  break;
		}
            }
          // match failed.
        }
      last = ch;
    }
  // Failure: we either reached EOF or experienced a read error.
  if (errno)
    return cssc::make_failure_from_errno(errno);
  return cssc::make_failure(cssc::errorcode::InternalErrorNoEncodedFlagFound);
}


static Failure
maybe_sync(FILE *fp)
{
#ifdef CONFIG_SYNC_BEFORE_REOPEN
  if (ffsync(fp) == EOF)
    return cssc::make_failure_from_errno(errno);
#else
  (void) &fp;                           // use the otherwise-unused parameter.
#endif
  return cssc::Failure::Ok();
}


/* End the update of the SCCS file by updating the checksum, and
   renaming the x-file to replace the old SCCS file. */

Failure
sccs_file::end_update(FILE **pout)
{
  Failure real_result = cssc::Failure::Ok();
  ResourceCleanup pout_closer([&pout, &real_result](){
      if (*pout != NULL)
	{
	  real_result = cssc::Update(real_result, fclose_failure(*pout));
	  *pout = NULL;
	}
    });

  const std::string xname = name_.xfile();
  auto diagnose = [xname](Failure f) -> cssc::FailureBuilder
    {
      return cssc::FailureBuilder(f).diagnose()
      << "failed to update " << xname;
    };

  // We execute the rest of end_update() inside a lambda so that we
  // can adjust real_result if we fail to close *pout.
  real_result = cssc::Update(real_result, [this, xname, &pout, diagnose]() -> cssc::Failure {
      auto write_error = [xname](int saved_errno)
	{
	  return cssc::make_failure_builder_from_errno(saved_errno)
	  .diagnose() << "failed to write to " << xname;
	};

      Failure result = fflush_failure(*pout);
      if (!result.ok())
	return diagnose(result) << "failed to flush " << xname;

      result = cssc::Update(result, maybe_sync(*pout));
      if (!result.ok())
	return diagnose(result) << "failed to sync " << xname;

      int sum;
      // Open the file (obtaining the checksum) and immediately close it.
      {
	auto opts = ParserOptions().set_silent_checksum_error(true);
	auto open_result = sccs_file_parser::open_sccs_file(xname, READ, opts);
	if (!open_result.ok())
	  return diagnose(open_result.fail()) << "failed to open " << xname;
	sum = (*open_result)->computed_sum;
      }


      // For "admin -i", we may need to change the "encoded" flag
      // from 0 to 1, if we found out that the input file was
      // binary, but the "-b" command line option had not been
      // given.  The checksum is adjusted if required.
      if (flags.encoded)
	{
	  rewind(*pout);
	  // TODO: change retrn type of rehack_encoded_flag.
	  Failure hacked = rehack_encoded_flag(*pout, &sum);
	  if (!hacked.ok())
	    return diagnose(hacked) << "failed to update encoded flag in "
				    << name_.xfile();
	}


      rewind(*pout);
      if (printf_failed(fprintf(*pout, "\001h%05d", sum)))
	{
	  return write_error(errno);
	}
      if (fclose_failed(fclose(*pout)))
	{
	  *pout = NULL;		// don't attempt to close it again.
	  return cssc::make_failure_builder_from_errno(errno)
	    .diagnose() << "failed to close " << xname;
	}
      *pout = NULL;

      /* JY, 2001-08-27: Under Windows we cannot rename or delete an open
       * file, so we close both the x-file and the s-file here in end_update().
       * I think closing the file here is harmless for all platforms, but
       * for the moment I will make it conditional.
       *
       * The destructor sccs_file::~sccs_file() asserts that the file pointer
       * is not NULL, so we reopen the file in this case.
       */
#if defined __CYGWIN__
      if (f)
	{
	  /* Only in modes other than create, will f be non-NULL */
	  fclose(f);
	}
#endif

      cssc::Failure retval = cssc::Failure::Ok();

      if (mode_ != CREATE && remove(name_.c_str()) == -1)
	{
	  return cssc::make_failure_builder_from_errno(errno)
	    .diagnose() << "failed to remove " << name_.c_str();
	}
      else if (rename(xname.c_str(), name_.c_str()) == -1)
	{
	  return cssc::make_failure_builder_from_errno(errno)
	    .diagnose() << "failed to rename " << xname.c_str()
			<< " to " << name_.c_str();
	}
      else
	{
	  xfile_created_ = false;    // What was the x-file is now the new s-file.
	  retval = cssc::Failure::Ok();
	}

#if defined __CYGWIN__
      int dummy_sum;

      std::string sfile_name = name_.sfile();
      // We will get a compilation error on this next line now,
      // because the type of open_sccs_file has changed.  But
      // I don't have access to a Cygwin system for testing.
      f = open_sccs_file(sfile_name.c_str(), READ, &dummy_sum);
      if (0 == f)
	{
	  s_missing_quit("Cannot re-open SCCS file %s for reading", name_.c_str());
	  retval = cssc::make_failure_from_errno(errno);
	}
#endif
      return retval;
    }());
  return real_result;
}



bool
sccs_file::update_checksum()
{
  return update();
}


/* Update the SCCS file */
bool
sccs_file::update()
{
  ASSERT(mode_ != CREATE);
  ASSERT(body_scanner_);

  if (!body_scanner_->seek_to_body().ok())
    {
      return false;
    }

  cssc::FailureOr<FILE*> fof = start_update();
  if (!fof.ok())
    return false;               // don't start writing the x-file.
  FILE *out = *fof;

  if (!write(out).ok())
    {
      xfile_error("Write error.");
      return false;
    }

  // assume that since the earlier seek_to_body() worked,
  // this one will too.
  if (!body_scanner_->seek_to_body().ok())
    return false;
  Failure copied = body_scanner_->copy_to(out);
  if (!copied.ok())
    {
      std::string msg = "write error on " + name_.xfile() + ": "
	+ copied.to_string();
      xfile_error(msg.c_str());
      return false;
    }
  return end_update(&out).ok();	// TODO: change return type
}

/* Local variables: */
/* mode: c++ */
/* End: */
