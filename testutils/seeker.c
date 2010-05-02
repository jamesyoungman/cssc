/* seeker.c; part of GNU CSSC.
 * 
 *    Copyright (C) 1998,2007 Free Software Foundation, Inc. 
 * 
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *    
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *    
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * Program that determines if the target system can seek on its standard
 * input.
 */
#include "config.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>



#define ERR_USAGE (1)

void usage(const char *name, int retval);

#ifdef HAVE_FSETPOS
static void
try_getpos(FILE *f)
{
  fpos_t pos;
  int rv;

  rv = fgetpos(f, &pos);
  printf("fgetpos() returns %d\n", rv);
  rv = fsetpos(f, &pos);
  printf("fsetpos() returns %d\n", rv);
}
#endif

static void
try_fseek(FILE *f)
{
  long lrv;
  int rv;
  
  lrv = ftell(f);
  printf("ftell() returns %ld\n", lrv);
  rv = fseek(f, SEEK_SET, lrv);
  printf("fseek() returns %d\n", rv);
}

static void
try_lseek()
{
  off_t pos;
  pos = lseek(0, 0, SEEK_CUR);
  printf("lseek() returns %ld [SEEK_CUR]\n", (long)pos);
  pos = lseek(0, pos, SEEK_CUR);
  printf("lseek() returns %ld [SEEK_SET]\n", (long)pos);
}


int do_help(const char *name)
{
  usage(name, 0);
  return 0;
}

int do_unbuffered(const char *name)
{
  (void) name;
  printf("stdin is unbuffered\n");
  setvbuf(stdin, (char*)NULL, _IONBF, 0u);
  return 0;
}

int do_blockbuffered(const char *name)
{
  (void) name;
  printf("stdin is fully-buffered\n");
  setvbuf(stdin, (char*)NULL, _IOFBF, BUFSIZ);
  return 0;
}

int do_linebuffered(const char *name)
{
  (void) name;
  printf("stdin is line-buffered\n");
  setvbuf(stdin, (char *)NULL, _IOLBF, 0);
  return 0;
}

int do_nothing(const char *name)
{
  (void) name;
  printf("stdin is buffered in the default way\n");
  return 0;
}


struct optact
{
  const char *option;
  int (*action)(const char*);
};
struct optact actions[]=
{
  { "--help", do_help },
  { "--unbuffered", do_unbuffered },
  { "--fully-buffered", do_blockbuffered },
  { "--line-buffered", do_linebuffered },
  { "--default-buffering", do_nothing }
};
#define NOPTIONS (sizeof(actions)/sizeof(actions[0]))



void usage(const char *name, int retval)
{
  unsigned int j;
  
  fprintf(stderr, "usage: %s ", name);
  for (j=0u; j<NOPTIONS; ++j)
    fprintf(stderr, "[%s] ", actions[j].option);
  fprintf(stderr, "\n");
  exit(retval);
}

int main(int argc, char *argv[])
{
  int i;
  unsigned int j;

  
  for (i=1; i<argc; ++i)
    {
      if ('-' == argv[i][0])
        {
          for (j=0u; j<NOPTIONS; ++j)
            {
              if (0 == strcmp(argv[i], actions[j].option))
                {
                  (*actions[j].action)(argv[0]);
                  break;
                }
            }
          
          if (NOPTIONS == j)
            {
              fprintf(stderr, "Unknown option `%s'\n", argv[i]);
              usage(argv[0], ERR_USAGE);        /* unknown option */
            }
        }
      else                      /* not an option */
        {
          usage(argv[0], ERR_USAGE);
        }
    }
#ifdef HAVE_FSETPOS
  try_getpos(stdin);
#else
  printf("fgetpos() not supported.\n");
#endif
  try_fseek(stdin);
  try_lseek();
  return 0;
}
