/*
 * sf-write.cc: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1998,1999,2001 Free Software Foundation, Inc. 
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA.
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
#include "delta.h"
#include "delta-iterator.h"
#include "linebuf.h"
#include "filepos.h"

#ifdef CONFIG_SCCS_IDS
static const char rcs_id[] = "CSSC $Id: sf-write.cc,v 1.32 2002/04/04 19:32:05 james_youngman Exp $";
#endif

/* Quit because an error related to the x-file. */
void
sccs_file::xfile_error(const char *msg) const
{
  errormsg_with_errno("%s: %s", name.xfile().c_str(), msg);
}


/* Start the update of the SCCS file by creating the x-file which will
   become the new SCCS file and writing a dummy checksum line to it. */

FILE *
sccs_file::start_update() {
        ASSERT(mode != READ);

        if (mode == CREATE && file_exists(name.c_str()))
          {
            errormsg("%s: SCCS file already exists.", name.c_str());
            return NULL;
          }
                     
        mystring xname = name.xfile();

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
            mystring newname(xname + ".bak");
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
        

        FILE *out = fcreate(xname, CREATE_READ_ONLY | CREATE_FOR_UPDATE);

        if (out == NULL)
          {
            xfile_error("Can't create temporary file for update.");
            return NULL;
          }
        else
          {
            xfile_created = true;
            
            if (fputs_failed(fputs("\001h-----\n", out)))
              {
                xfile_error("write error");
                fclose(out);
                return NULL;
              }
            return out;
          }
}


static int
print_seqs(FILE *out, char control, mylist<seq_no> const &seqs) {
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
sccs_file::write_delta(FILE *out, struct delta const &d) const
{
  int len;
  int i;

  if (printf_failed(fprintf(out, "\001s %05lu/%05lu/%05lu\n",
                            cap5(d.inserted),
                            cap5(d.deleted),
                            cap5(d.unchanged)))
      || printf_failed(fprintf(out, "\001d %c ", d.type))
      || d.id.print(out)
      || putc_failed(putc(' ', out))
      || d.date.print(out)
      || printf_failed(fprintf(out, " %s %u %u\n",
                               d.user.c_str(),
                               d.seq, d.prev_seq)))
    {
      return 1;
    }

  if (print_seqs(out, 'i', d.included)
      || print_seqs(out, 'x', d.excluded)
      || print_seqs(out, 'g', d.ignored))
    {
      return 1;
    }
  
  len = d.mrs.length();
  for(i = 0; i < len; i++)
    {
      if (printf_failed(fprintf(out, "\001m %s\n",
                                d.mrs[i].c_str())))
        {
          return 1;
        }
    }

  len = d.comments.length();
  for(i = 0; i < len; i++)
    {
      if (printf_failed(fprintf(out, "\001c %s\n",
                                d.comments[i].c_str())))
        {
          return 1;
        }
    }

  return fputs_failed(fputs("\001e\n", out));
}


/* Writes everything up to the body to new SCCS file.  Returns non-zero
   if an error occurs. */

int
sccs_file::write(FILE *out) const
{
  int len;
  int i;
  const char *s;

  delta_iterator iter(delta_table);
  while (iter.next(1)) {
    if (write_delta(out, *iter.operator->())) {
      return 1;
    }
  }

  if (fputs_failed(fputs("\001u\n", out))) {
    return 1;
  }

  len = users.length();
  for(i = 0; i < len; i++) {
    s = users[i].c_str();
    ASSERT(s[0] != '\001');
    if (printf_failed(fprintf(out, "%s\n", s))) {
      return 1;
    }
  }

  if (fputs_failed(fputs("\001U\n", out))) {
    return 1;
  }

  // Beginning of flags...

  // b branch
  if (flags.branch && fputs_failed(fputs("\001f b\n", out))) {
    return 1;
  }
                
  // c ceiling
  if (flags.ceiling.valid())
    {
      if (fputs_failed(fputs("\001f c ", out))
         || flags.ceiling.print(out)
         || putc_failed(putc('\n', out)))
        {
          return 1;
        }
    }
        
  // d default SID
  if (flags.default_sid.valid())
    {
      if (fputs_failed(fputs("\001f d ", out))
          || flags.default_sid.print(out)
          || putc_failed(putc('\n', out)))
        {
          return 1;
        }
    }
        
  // f floor
  if (flags.floor.valid())
    {
      if (fputs_failed(fputs("\001f f ", out))
          || flags.floor.print(out)
          || putc_failed(putc('\n', out)))
        {
          return 1;
        }
    }
        
  // i no id kw is error
  if (flags.no_id_keywords_is_fatal) {
    if (fputs_failed(fputs("\001f i\n", out))) {
      return 1;
    }
  }

  // j joint-edit
  if (flags.joint_edit)
    {
      if (fputs_failed(fputs("\001f j\n", out)))
        return 1;
    }

  // l locked-releases
  if (flags.all_locked)
    {
      if (fputs_failed(fputs("\001f l a\n", out)))
        {
          return 1;
        }
    } 
  else if ( !flags.locked.empty() )
    {
      if (fputs_failed(fputs("\001f l ", out)))
        return 1;               // failed
      
      if (!flags.locked.print(out))
        return 1;               // failed
      
      if (putc_failed(putc('\n', out)))
        return 1;               // failed
    }

  // m Module flag
  if (flags.module)
    {
      const char *p = flags.module->c_str();
      if (printf_failed(fprintf(out, "\001f m %s\n", p)))
        {
          return 1;
        }
    }
        
  // n Create empty deltas
  if (flags.null_deltas)
    {
      if (fputs_failed(fputs("\001f n\n", out)))
        return 1;
    }

  // q %Q% subst value (user flag)
  if (flags.user_def)
    {
      const char *p = flags.user_def->c_str();
      if (printf_failed(fprintf(out, "\001f q %s\n", p)))
        {
          return 1;
        }
    }
        
  // t %Y% subst value
  if (flags.type)
    {
      if (printf_failed(fprintf(out, "\001f t %s\n",
                                flags.type->c_str())))
        {
          return 1;
        }
    }
        
  // v MR-validation program.
  if (flags.mr_checker)
    {
      if (printf_failed(fprintf(out, "\001f v %s\n",
                                flags.mr_checker->c_str())))
        return 1;
    }

  // Write the correct valuie for the "encoded" flag.
  // We have to write it even if the flag is unset, 
  // because "admin -i" goes back and updates that byte if the file 
  // turns out to have been binary.
  if (printf_failed(fprintf(out, "\001f e %c",
                            (flags.encoded ? '1' : '0'))))
    return 1;

        
  if (printf_failed(fprintf(out, "\n")))
    return 1;
        
  if (flags.reserved)
    {
      if (printf_failed(fprintf(out, "\001f z %s\n",
                                flags.reserved->c_str()))) {
        return 1;
      }
    }
        
  // end of flags.

  if (fputs_failed(fputs("\001t\n", out)))
    {
      return 1;
    }

  len = comments.length();
  for(i = 0; i < len; i++) {
    s = comments[i].c_str();
    ASSERT(s[0] != '\001');
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



int
sccs_file::rehack_encoded_flag(FILE *f, int *sum) const
{
  // Find the encoded flag.  Maybe change it.
  // "f" must be opened for update.
  int ch, last;
  last = '\n';
  const char match[] = "\001f e ";
  const int nmatch = strlen(match);
  int n;
  
  while ( EOF != (ch=getc(f)) )
    {
      if ('\n' == last)
        {
          n = 0;
          while (match[n] == ch)
            {
              if (nmatch-1 == n)
                {
                  // success; now we might change the flag.
                  FilePosSaver *pos = new FilePosSaver(f);
                  ch = getc(f);
                  if ('0' == ch)
                    {
                      delete pos; // rewind file 1 char.
                      putc('1', f);
                      const int d =  ('1' - '0'); // change to checksum.
                      *sum = (*sum + d) & 0xFFFF; // adjust checksum.
                      return 0;
                    }
                  else
                    {
                      pos->disarm();
                      delete pos;
                      return 0; // flag was already set.
                    }
                }
              ++n;              // advance match.
              ch = getc(f);
            }
          // match failed.
        }
      last = ch;
    }
  return 1;                     // failed!
}


static bool
maybe_sync(FILE *fp)
{
#ifdef CONFIG_SYNC_BEFORE_REOPEN
  if (ffsync(fp) == EOF)
    return false;
#else
  (void) &fp;                           // use the otherwise-unused parameter.
#endif
  return true;
}


/* End the update of the SCCS file by updating the checksum, and
   renaming the x-file to replace the old SCCS file. */

bool
sccs_file::end_update(FILE **pout)
{
  if (fflush_failed(fflush(*pout)) || !maybe_sync(*pout))
    {
      fclose(*pout);
      *pout = NULL;
      xfile_error("Write error.");
    }

  int sum;
  mystring xname = name.xfile();
  
  // Open the file (obtaining the checksum) and immediately close it.
  if (fclose_failed(fclose(open_sccs_file(xname.c_str(), READ, &sum))))
    xfile_error("Error closing file.");
  
  // For "admin -i", we may need to change the "encoded" flag
  // from 0 to 1, if we found out that the input file was
  // binary, but the "-b" command line option had not been
  // given.  The checksum is adjusted if required.
  if (flags.encoded)
    {
      rewind(*pout);
      if (0 != rehack_encoded_flag(*pout, &sum))
        {
          xfile_error("Write error.");
        }
    }
  
  
  rewind(*pout);
  if (printf_failed(fprintf(*pout, "\001h%05d", sum)))
    {
      (void) fclose(*pout);
      *pout = NULL;
      xfile_error("Write error.");
    }
  if (fclose_failed(fclose(*pout)))
    {
      *pout = NULL;
      xfile_error("Write error.");
    }

  /* JY, 2001-08-27: Under Windows we cannot rename or delete an open 
   * file, so we close both the x-file and the s-file here in end_update().
   * I think closing the file here is harmless for all platforms, but 
   * for the moment I will make it conditional.
   *
   * The destructor sccs_file::~sccs_file() asserts that the file pointer
   * is not NULL, so we reopen the file in this case.
   */
#if defined(__CYGWIN__)
  fclose(f);
#endif

  bool retval = false;
  
  if (mode != CREATE && remove(name.c_str()) == -1)
    {
      errormsg_with_errno("%s: Can't remove old SCCS file.", name.c_str());
      retval = false;
    }
  else if (rename(xname.c_str(), name.c_str()) == -1)
    {
      xfile_error("Can't rename new SCCS file.");
      retval = false;
    }
  else
    {
      xfile_created = false;    // What was the x-file is now the new s-file.
      retval = true;
    }
  
#if defined(__CYGWIN__)
  int dummy_sum;
  mystring sfile_name = name.sfile();
  f = open_sccs_file(sfile_name.c_str(), READ, &dummy_sum);
  if (0 == f)
    {
      s_missing_quit("Cannot re-open SCCS file %s for reading", name.c_str());
      retval = false;
    }
#endif
  return retval;
}



/* Recalculate and update the checksum of a SCCS file. */
#if 0
bool
sccs_file::update_checksum(const char *name)
{
  int sum;
  FILE *out;

  if (NULL == (out = open_sccs_file(name, UPDATE, &sum)))
    return false;

  if (fprintf_failed(fprintf(out, "\001h%05d", sum)))
    {
      errormsg_with_errno("%s: Write error", name);
      fclose(out);
      return false;
    }
  else
    {
      if (fclose_failed(fclose(out)))
        {
          errormsg_with_errno("%s: Write error", name);
          return false;
        }
    }
  return true;
}
#else
// this version is not a static function.
bool
sccs_file::update_checksum()
{
  return update();
}
#endif


/* Update the SCCS file */

bool
sccs_file::update()
{
  ASSERT(mode != CREATE);
  
  if (!seek_to_body())
    return false;
  
  FILE *out = start_update();
  if (NULL == out)
    return false;               // don't start writing the x-file.
  
  if (write(out))
    {
      xfile_error("Write error.");
    }

  // assume that since the earlier seek_to_body() worked,
  // this one will top.
  if (!seek_to_body())
    return false;
  
  while (read_line() != -1)
    {
      if (fputs_failed(fputs(plinebuf->c_str(), out))
          || putc_failed(putc('\n', out)))
        {
          xfile_error("Write error.");
        }
    }
  
  return end_update(&out);
}

/* Local variables: */
/* mode: c++ */
/* End: */
