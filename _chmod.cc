/*
 * _chmod.cc: Part of GNU CSSC.
 * 
 *    Copyright (C) 1997, Free Software Foundation, Inc. 
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
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 * $Id: _chmod.cc,v 1.6 1998/01/24 14:11:39 james Exp $
 *
 */

#ifndef CSSC___CHMOD_C__
#define CSSC___CHMOD_C__

#define _chmod LIDENT(_chmod)

#ifdef CONFIG_DJGPP

/* Gets or sets a file's attributes under MS-DOS. */

int
_chmod(const char *name, int fl, int attr = 0) {
	char ret;
	short err;

	register short attrib asm("%ecx") = attr;
	register char flag asm("%eax") = (fl != 0);

	asm volatile ("mov $0x43, %%ah;"
		      "int $0x21;"
		      "setb %2;"
		      : "=a" (err), "=c" (attrib), "=g" (ret)  /* output */
		      : "0" (flag), "1" (attrib), "d" (name)   /* input */
		      : "cc", "%ebx", "%edx", "%esi", "%edi"); /* clobbered */

	if (ret) {
		errno = err;
		return -1;
	}
	return attrib;
}

#else /* CONFIG_DJGPP */


/* Gets or sets a file's attributes under MS-DOS. */

int
_chmod(const char *name, int fl, int attr = 0) {
	union REGS regs;

	regs.x.cx = (short) attr;
	regs.h.al = (char) fl;
	regs.h.ah = 0x43;

#ifdef FP_SEG
	struct SREGS regs;
	sregs.ds = FP_SEG(name);
	regs.x.dx = FP_OFF(name);
	intdosx(&regs, &regs, &sregs);
#else
	regs.x.dx = name;
	intdos(&regs, &regs);
#endif

	if (regs.x.cflag) {
		errno = regs.x.ax;
		return -1;
	}
	return regs.x.cx;
}

#endif /* CONFIG_DJGPP */

#endif /* CSSC___CHMOD_C__ */

/* Local variables: */
/* mode: c++ */
/* End: */
