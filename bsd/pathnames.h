/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Copyright (C) 1998, Free Software Foundation, Inc.
 *      
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)pathnames.h	8.1 (Berkeley) 6/6/93
 *
 * Note that this is not the original version of the file.  It has 
 * been modified but of course still keeps the same license.
 */

#include <paths.h>

/* #define PASTE(a,b) a##b */

#define	_PATH_SCCSADMIN	("admin")
#define	_PATH_SCCSBDIFF	("bdiff")
#define	_PATH_SCCSCOMB	("comb")
#define	_PATH_SCCSDELTA	("delta")
#define	_PATH_SCCSDIFF	("sccsdiff")
#define	_PATH_SCCSGET	("get")
#define	_PATH_SCCSHELP	("help")
#define	_PATH_SCCSPRS	("prs")
#define	_PATH_SCCSPRT	("prt")
#define	_PATH_SCCSRMDEL	("rmdel")
#define	_PATH_SCCSVAL	("val")
#define	_PATH_SCCSWHAT	("what")
#undef _PATH_TMP
#define	_PATH_TMP	"/tmp/sccsXXXXX"
