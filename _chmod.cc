/*
 * _chmod.c
 *
 * By Ross Ridge
 * Public Domain
 *
 * Defines the function _chmod for MS-DOS systems.
 *
 * $Id: _chmod.cc,v 1.3 1997/05/10 14:49:48 james Exp $
 *
 */

#ifndef ___CHMOD_C__
#define ___CHMOD_C__

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

#endif /* ___CHMOD_C__ */

/* Local variables: */
/* mode: c++ */
/* End: */
