/*
 * fsync.c
 * 
 * By Ross Ridge
 * Public Domain
 *
 * Defines the function fsync for MS-DOS systems.
 *
 * @(#) MySC fsync.c 1.1 93/11/09 17:17:54
 *
 */

#ifndef __FSYNC_H__
#define __FSYNC_H___

#ifndef CONFIG_DJGPP

struct check_msdos_version {
	check_msdos_version();
};

check_msdos_version::check_msdos_version() {
	if (_osmajor < 2 || (_osmajor == 3 && _osminor < 3)) {
		static char const msg[] = "MS-DOS version 3.3 or later"
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
