/*
 * fsync.cc: Part of GNU CSSC.
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA.
 * 
 * CSSC was originally Based on MySC, by Ross Ridge, which was 
 * placed in the Public Domain.
 *
 *
 * Defines the function fsync for MS-DOS systems.
 *
 */

#ifndef CSSC__FSYNC_H__
#define CSSC__FSYNC_H___

#ifndef CONFIG_DJGPP

struct check_msdos_version {
	check_msdos_version();
};

check_msdos_version::check_msdos_version() {
	if (_osmajor < 2 || (_osmajor == 3 && _osminor < 3)) {
		static const char msg[] = "MS-DOS version 3.3 or later"
			                  " required.\r\n";
		_write(2, msg, sizeof(msg));
	}
}

static class check_msdos_version check_msdos_version;

#endif

#define fsync LIDENT(fsync) 

/* Updates the files contents and most importantly the file's directory
   entry on to disk. */

static int
fsync(int fd) {
#ifdef __BORLANDC__

	asm mov bx, fd;
	asm mov ah, 0x68;
	asm int 21h;
	asm jnc ok;
	asm mov errno, ax;
	return -1;
ok:
	return 0;

#else /* __BORLANDC__ */
#ifdef __GNUC__
	char ret;
	short err;

	asm volatile ("movb $0x68, %%ah;"
		      "int $0x21;"
		      "setb %0;"
		      : "=g" (ret), "=a" (err)	/* output */
		      : "b" ((short) fd)	/* input */
		      : "cc", "%ebx", "%ecx", "%edx", "%esi", "%edi");
	if (ret) {
		errno = err;
		return -1;
	}
	return 0;

#else /* __GNUC__ */	     
	
	union REGS regs;
	regs.x.bx = (short) fd;
	regs.h.ah = 0x68;
	intdos(&regs, &regs);
	if (regs.x.cflag) {
		errno = regs.x.ax;
		return -1;
	}
	return 0;

#endif /* __GNUC__ */
#endif /* __BORLANDC__ */
}

#endif /* __FSYNC_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
