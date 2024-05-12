/*
 * fatalsig.cc: Part of GNU CSSC.
 *
 *
 *  Copyright (C) 2000, 2001, 2007, 2008, 2009, 2010, 2011, 2014, 2019,
 *  2024 Free Software Foundation, Inc.
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
 *
 * Handle fatal signals...
 *
 */
#include "config.h"

#include <cstdlib>
#include <cstring>
#include <map>
#include <vector>
#include <signal.h>             /* TODO: consider using sigaction(). */

#include "cssc-assert.h"
#include "cssc.h"
#include "quit.h"
#include "version.h"

namespace
{

/* The expansion of RETSIGTYPE is automatically decided by the configure
 * script; its value is define in config.h.
 */
typedef RETSIGTYPE (*sighandler)(int);


/* fatal_signals_to_trap contains a list of signals whose
 * receipt is normally fatal and for which we wish to perform
 * cleanup.
 */
const std::vector<int> fatal_signals_to_trap =
{
  SIGHUP, SIGINT, SIGQUIT, SIGPIPE
};

/* In the array set_sig_handlers() we keep the initial disposition
 * of the signals whose dispositions we alter, in order to
 * restore them inside the signal handler.
 */
std::map<int, sighandler> initial_dispositions;

/* set_sig_handlers
 *
 * This function sets up signal handers for various fatal signals and
 * remembers the original settings.
 */
void set_sig_handlers( sighandler pfn )
{
  for (int sig : fatal_signals_to_trap)
    {
      struct sigaction action, oa;
      memset(&oa, 0, sizeof(oa));
      memset(&action, 0, sizeof(action));
      action.sa_handler = pfn;
      if (0 != sigaction(sig, &action, &oa))
        {
          /* This is a nonfatal error. */
          errormsg_with_errno("Failed to set signal handler for signal %d", sig);
          oa.sa_handler = SIG_DFL;
        }
      initial_dispositions[sig] = oa.sa_handler;
    }
}

/* reset_sig_handlers
 *
 * Resets the original signal handlers for the fatal signals
 * as saved in initial_dispositions.
 */
void reset_sig_handlers( void )
{
  for (const auto mapping : initial_dispositions)
    {
      struct sigaction action;
      memset(&action, 0, sizeof(action));
      action.sa_handler = mapping.second;
      if (0 != sigaction(mapping.first, &action, nullptr))
	{
          /* This is a nonfatal error. */
          errormsg_with_errno("Failed to reset signal handler for signal %d",
			      mapping.first);
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

}  // unnamed namespace

void quit_on_fatal_signals (void)
{
  static int already_called = 0;
  ASSERT(already_called == 0);

  set_sig_handlers(handle_fatal_sig);

  already_called = 1;
}
