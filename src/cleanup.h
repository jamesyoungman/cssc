/*
 * cleanup.h: Part of GNU CSSC.
 * 
 * 
 *    Copyright (C) 1997,1999,2001,2007 Free Software Foundation, Inc. 
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
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 */
#ifndef CSSC__CLEANUP_H__
#define CSSC__CLEANUP_H__

// Cleaner is a class that you should have in scope in main;
// it ensures that all required cleanups are run even if you don't
// call quit().
class Cleaner
{
public:
  Cleaner();
  ~Cleaner();
};

/*
 * The class "cleanup" is one which you can inherit from in order to 
 * ensure that resources are cleaned up when a function is exited.
 * This is most notably used by the "file_lock" class.
 */
class cleanup {
        static class cleanup *head;
        static int running;
        static int all_disabled;
#if HAVE_FORK
        static int in_child_flag;
#endif

        class cleanup *next;
        
protected:
        cleanup();
        virtual void do_cleanup() = 0;

#ifdef __GNUC__
        virtual /* not needed but it gets rid of an anoying warning */
#endif
        ~cleanup();

public:

        static void run_cleanups();
        static int active() { return running; }
        static void disable_all() { all_disabled++; }
#ifdef HAVE_FORK
        static void set_in_child() { in_child_flag = 1; disable_all(); }
        static int in_child() { return in_child_flag; }
#endif
};

#endif
