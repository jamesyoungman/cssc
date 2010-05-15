/*
 * fatalsig.cc: Part of GNU CSSC.
 *
 *
 *    Copyright (C) 2000,2001,2007 Free Software Foundation, Inc.
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
 *
 *
 * Handle fatal signals...
 *
 */
#include <cstdlib>

#include "cssc.h"
#include "version.h"
#include "quit.h"

#include <signal.h>             /* TODO: consider using sigaction(). */
#include "cssc-assert.h"


/* The expansion of RETSIGTYPE is automatically decided by the configure
 * script; its value is define in config.h.
 */
typedef RETSIGTYPE (*sighandler)(int);


/* fatal_signals_to_trap contains a list of signals whose
 * receipt is normally fatal and for which we wish to perform
 * cleanup.
 */
static const int fatal_signals_to_trap[] =
{
  SIGHUP, SIGINT, SIGQUIT, SIGPIPE
};
#define N_TRAPPED_SIGS (sizeof(fatal_signals_to_trap) /   \
                        sizeof(fatal_signals_to_trap[0]))

/* In the array set_sig_handlers() we keep the initial disposition
 * of the signals whose dispositions we alter, in order to
 * restore them inside the signal handler.
 */
static sighandler initial_dispositions[N_TRAPPED_SIGS];
static int already_called = 0;


/* set_sig_handlers
 *
 * This function sets up signal handers for various fatal signals and
 * remembers the original settings.
 */
static void set_sig_handlers( sighandler pfn )
{
  unsigned int i;

  for (i=0u; i<N_TRAPPED_SIGS; ++i)
    {
      int sig = fatal_signals_to_trap[i];
      sighandler orighand = signal(sig, pfn);

      if (SIG_ERR == orighand)
        {
          /* This is a nonfatal error. */
          errormsg_with_errno("Failed to set signal handler for signal %d",
                              sig);
          initial_dispositions[i] = SIG_DFL;
        }
      else
        {
          initial_dispositions[i] = orighand;
        }
    }
}

/* reset_sig_handlers
 *
 * Resets the original signal handlers for the fatal signals
 * as saved in the array initial_dispositions.
 */
static void reset_sig_handlers( void )
{
  unsigned int i;

  for (i=0u; i; ++i)
    {
      int sig = fatal_signals_to_trap[i];

      if (SIG_ERR == signal(sig, initial_dispositions[i]))
        {
          /* This is a nonfatal error. */
          errormsg_with_errno("Failed to reset signal handler for signal %d",
                              sig);
        }
    }
}


/* handle_fatal_sig
 *
 * This is the signal handler function for fatal signals.
 * It does some cleanup and exits the program.
 */
static RETSIGTYPE
handle_fatal_sig(int /* whatsig */ )
{
  /* Reset the signal handler to the original setting. */
  /* Also reset all the others too.  This is in order to
   * handle a "stuck" program in a useful way, and helps to
   * ensure that the cleanup operation is itself interruptible
   */
  reset_sig_handlers();

  /* The cleanups will delete any existing lockfile.
   * However, at the moment we do not delete an incomplete
   * g-file.
   */
  cleanup::run_cleanups();

  /* Here, we exit with exit() rather than _exit().   Experience
   * may show that that is a bad idea.
   */
  exit(1);
}


void quit_on_fatal_signals (void)
{
  ASSERT(already_called == 0);

  set_sig_handlers(handle_fatal_sig);

  already_called = 1;
}
