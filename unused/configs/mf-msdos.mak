#
# makefile
#
# By Ross Ridge
# Public Domain
#
# Makefile for compiling the MySC utilities under MS-DOS with Borland's
# MAKE utility.
#
# @(#) MySC mf-msdos.mm 1.6 93/12/31 15:14:34
# @(#) MySC mf-inc.mm 1.4 93/11/09 23:15:38
#

CC = bcc
CFLAGS = -ms -1 -a -b- -d -f- -v -A -w -P -Od
LDFLAGS = -ms -f- -v -M -ls -L.
#C0OBJ =
#LIBS =

GCC = gcc
GCFLAGS = -Wall -Wpointer-arith -Wwrite-strings -Wmissing-prototypes\
	  -Wconversion -Wcast-align -Wstrict-prototypes -Wnested-externs\
	  -Winline -Wno-comment -x c++ -fno-omit-frame-pointer -O
GLDFLAGS =

CMDS = get.exe delta.exe admin.exe prs.exe what.exe unget.exe sact.exe \
	cdc.exe rmdel.exe
GCMDS = $(CMDS:.exe=)
XCMDS = $(CMDS:.exe=.x)

GETOBJS = get.obj quit.obj xalloc.obj mystring.obj sccsname.obj sid.obj \
	sccsdate.obj linebuf.obj file.obj split.obj getopt.obj fileiter.obj \
	sccsfile.obj sf-get.obj sf-get2.obj sf-get3.obj sf-chkid.obj \
	pfile.obj pf-add.obj
DELTAOBJS = delta.obj quit.obj xalloc.obj mystring.obj sccsname.obj sid.obj \
	sccsdate.obj linebuf.obj file.obj split.obj getopt.obj fileiter.obj \
	sccsfile.obj sf-delta.obj sf-get.obj sf-get3.obj sf-chkid.obj \
	sf-write.obj sf-add.obj pfile.obj pf-del.obj pipe.obj run.obj \
	l-split.obj prompt.obj
ADMINOBJS = admin.obj quit.obj xalloc.obj mystring.obj sccsname.obj sid.obj \
	sccsdate.obj linebuf.obj file.obj split.obj getopt.obj fileiter.obj \
	sccsfile.obj sf-admin.obj sf-chkid.obj sf-write.obj sf-add.obj \
	run.obj l-split.obj prompt.obj
PRSOBJS = prs.obj quit.obj xalloc.obj mystring.obj sccsname.obj sid.obj \
	sccsdate.obj linebuf.obj file.obj split.obj getopt.obj fileiter.obj \
	sccsfile.obj sf-prs.obj sf-get.obj sf-chkid.obj
WHATOBJS = what.obj quit.obj getopt.obj
UNGETOBJS = unget.obj quit.obj xalloc.obj mystring.obj sccsname.obj sid.obj \
	sccsdate.obj linebuf.obj file.obj split.obj getopt.obj fileiter.obj \
	pfile.obj pf-del.obj
SACTOBJS = sact.obj quit.obj xalloc.obj mystring.obj sccsname.obj sid.obj \
	sccsdate.obj linebuf.obj file.obj split.obj getopt.obj fileiter.obj \
	pfile.obj
CDCOBJS = cdc.obj quit.obj xalloc.obj mystring.obj sccsname.obj sid.obj \
	sccsdate.obj linebuf.obj file.obj split.obj getopt.obj fileiter.obj \
	sccsfile.obj sf-cdc.obj sf-write.obj run.obj l-split.obj prompt.obj
RMDELOBJS = rmdel.obj quit.obj xalloc.obj mystring.obj sccsname.obj sid.obj \
	sccsdate.obj linebuf.obj file.obj split.obj getopt.obj fileiter.obj \
	sccsfile.obj sf-rmdel.obj sf-write.obj pfile.obj

SRCS = admin.c cdc.c delta.c file.c fileiter.c get.c getopt.c l-split.c \
	linebuf.c mystring.c pf-add.c pf-del.c pfile.c pipe.c prompt.c prs.c \
	quit.c rmdel.c run.c sact.c sccsdate.c sccsfile.c sccsname.c \
	sf-add.c sf-admin.c sf-cdc.c sf-chkid.c sf-delta.c sf-get.c \
	sf-get2.c sf-get3.c sf-prs.c sf-rmdel.c sf-write.c sid.c split.c \
	unget.c what.c xalloc.c
INCLUDES = defaults.h file.h fileiter.h filelock.h getopt.h linebuf.h \
	list.h mysc.h mystring.h pfile.h pipe.h quit.h run.h sccsdate.h \
	sccsfile.h sccsname.h seqstate.h sf-chkmr.h sid.h sid_list.h stack.h \
	sysdep.h xalloc.h fsync.c _chmod.c strstr.c list.c sid_list.c \
	sl-merge.c
XSRCS = $(CMDS:.exe=.cs)

.autodepend
.SUFFIXES: 
.SUFFIXES: .o .c .s .i .x .cs

.c.obj:
	$(CC) $(CFLAGS) -c{ $<}

.c.o: 
	$(GCC) @gcc.cfl -c{ $<}

.cs.x:
	$(GCC) @gcc.cfl $(GLDFLAGS) -o $@ $<

.c.s:
	$(GCC) @gcc.cfl -S $<

.c.i:
	$(GCC) @&&!
$(GCFLAGS)
! -E $< > $@

it: $(CMDS)

gcc: gcc.cfl $(GCMDS)

xgcc: gcc.cfl $(XCMDS)

all: it gcc


get.exe: $(C0OBJ) $(GETOBJS)
	$(CC) $(LDFLAGS) -eget.exe @&&!
$(GETOBJS) $(LIBS)
!

delta.exe: $(C0OBJ) $(DELTAOBJS)
	$(CC) $(LDFLAGS) -edelta.exe @&&!
$(DELTAOBJS) $(LIBS)
!

admin.exe: $(C0OBJ) $(ADMINOBJS)
	$(CC) $(LDFLAGS) -eadmin.exe @&&!
$(ADMINOBJS) $(LIBS)
!

prs.exe: $(C0OBJ) $(PRSOBJS)
	$(CC) $(LDFLAGS) -eprs.exe @&&!
$(PRSOBJS) $(LIBS)
!

what.exe: $(C0OBJ) $(WHATOBJS)
	$(CC) $(LDFLAGS) -ewhat.exe @&&!
$(WHATOBJS) $(LIBS)
!

unget.exe: $(C0OBJ) $(UNGETOBJS)
	$(CC) $(LDFLAGS) -eunget.exe @&&!
$(UNGETOBJS) $(LIBS)
!

sact.exe: $(C0OBJ) $(SACTOBJS)
	$(CC) $(LDFLAGS) -esact.exe @&&!
$(SACTOBJS) $(LIBS)
!

cdc.exe: $(C0OBJ) $(CDCOBJS)
	$(CC) $(LDFLAGS) -ecdc.exe @&&!
$(CDCOBJS) $(LIBS)
!

rmdel.exe: $(C0OBJ) $(RMDELOBJS)
	$(CC) $(LDFLAGS) -ermdel.exe @&&!
$(RMDELOBJS) $(LIBS)
!


sl-test.exe: $(C0OBJ) sid.h sid.c sid_list.h sid_list.c
	$(CC) $(CFLAGS) $(LDFLAGS) -DTEST -esl-test sid_list.c

get: $(GETOBJS:.obj=.o)
	$(GCC) $(GLDFLAGS) -o get @&&!
$(GETOBJS:.obj=.o)
!

delta: $(DELTAOBJS:.obj=.o)
	$(GCC) $(GLDFLAGS) -o delta @&&!
$(DELTAOBJS:.obj=.o)
!

admin: $(ADMINOBJS:.obj=.o)
	$(GCC) $(GLDFLAGS) -o admin @&&!
$(ADMINOBJS:.obj=.o)
!

prs: $(PRSOBJS:.obj=.o)
	$(GCC) $(GLDFLAGS) -o prs @&&!
$(PRSOBJS:.obj=.o)
!

what: $(WHATOBJS:.obj=.o)
	$(GCC) $(GLDFLAGS) -o what @&&!
$(WHATOBJS:.obj=.o)
!

unget: $(UNGETOBJS:.obj=.o)
	$(GCC) $(GLDFLAGS) -o unget @&&!
$(UNGETOBJS:.obj=.o)
!

sact: $(SACTOBJS:.obj=.o)
	$(GCC) $(GLDFLAGS) -o sact @&&!
$(SACTOBJS:.obj=.o)
!

cdc: $(CDCOBJS:.obj=.o)
	$(GCC) $(GLDFLAGS) -o cdc @&&!
$(CDCOBJS:.obj=.o)
!

rmdel: $(RMDELOBJS:.obj=.o)
	$(GCC) $(GLDFLAGS) -o rmdel @&&!
$(RMDELOBJS:.obj=.o)
!


test:
	-del test\passwd.$
	-del test\passwd.%
	-del test\passwd.&
	admin -itest\passwd.1 test\passwd.$
	get -e -g test\passwd.$
	copy test\passwd.2 passwd
	delta -Y test\passwd.$
	get -e -g test\passwd.$
	copy test\passwd.3 passwd
	delta -Y test\passwd.$
	get -e -g -x1.2 test\passwd.$
	copy test\passwd.4 passwd
	delta -Y test\passwd.$
	get -e -g test\passwd.$
	copy test\passwd.5 passwd
	delta -Y test\passwd.$
	get -e -g -r1.3 test\passwd.$
	copy test\passwd.6 passwd
	delta -Y test\passwd.$
	get -r1.1 -p test\passwd.$ > test\passwd.m1
	get -r1.2 -p test\passwd.$ > test\passwd.m2
	get -r1.3 -p test\passwd.$ > test\passwd.m3
	get -r1.4 -p test\passwd.$ > test\passwd.m4
	get -r1.5 -p test\passwd.$ > test\passwd.m5
	get -r1.3.1.1 -p test\passwd.$ > test\passwd.m6

gtest:
	-del test\passwd.$
	-del test\passwd.%
	-del test\passwd.&
	go32 admin -itest\passwd.1 test\passwd.$
	go32 get -e -g test\passwd.$
	copy test\passwd.2 passwd
	go32 delta -Y test\passwd.$
	go32 get -e -g test\passwd.$
	copy test\passwd.3 passwd
	go32 delta -Y test\passwd.$
	go32 get -e -g -x1.2 test\passwd.$
	copy test\passwd.4 passwd
	go32 delta -Y test\passwd.$
	go32 get -e -g test\passwd.$
	copy test\passwd.5 passwd
	go32 delta -Y test\passwd.$
	go32 get -e -g -r1.3 test\passwd.$
	copy test\passwd.6 passwd
	go32 delta -Y test\passwd.$
	go32 get -r1.1 -p test\passwd.$ > test\passwd.m1
	go32 get -r1.2 -p test\passwd.$ > test\passwd.m2
	go32 get -r1.3 -p test\passwd.$ > test\passwd.m3
	go32 get -r1.4 -p test\passwd.$ > test\passwd.m4
	go32 get -r1.5 -p test\passwd.$ > test\passwd.m5
	go32 get -r1.3.1.1 -p test\passwd.$ > test\passwd.m6
	
depend: $(SRCS) $(XSRCS) $(INCLUDES) config.h
	sed -e "/^# YOW\.$/p" -e "/^# YOW\.$/,$d" makefile > makefile.new
	gcc -MM @&&!
$(GCFLAGS) -MM $(SRCS) $(XSRCS)
! > makefile.tmp
	sed -e "s/\.cs\.o/.x/" -e "s/ :/:/" -e "s/ cf-djgpp\.h//" mf-deps.tmp >> makefile.new
	del makefile.tmp

gclean:
	-for %i in ($(GCMDS)) do del %i

clean: gclean
	-del *.map
	-del *.exe
	-del *.obj
	-del *.o
	-del *.x

gcc.cfl: $(MAKEFILE)
	copy &&!
$(GCFLAGS)
! gcc.cfl

MAKEFILE = makefile

## !DIST.
#
#GET = d:\bin\get
#C0OBJ = c0s.obj 
#LIBS = d:\borlandc\lib\wildargs.obj
#!ifndef SRCDIR
#SRCDIR = .
#!endif
#SCCSDIR = $(SRCDIR)\SCCS
#
#.SUFFIXES: .mak .mm .mm$
#
#.path.mm$ = $(SCCSDIR)
#.path.c$ = $(SCCSDIR)
#.path.h$ = $(SCCSDIR)
#.path.tx$ = $(SCCSDIR)
#
#.mm$.mm:
#	if not exist $@ $(GET) $<
#	touch $@
#	attrib -a $@
#
#.c$.c:
#	if not exist $@ $(GET) $<
#	touch $@
#	attrib -a $@
#
#.h$.h:
#	if not exist $@ $(GET) $<
#	touch $@
#	attrib -a $@
#
#.tx$.txt:
#	if not exist $@ $(GET) -p $< > txt.tmp
#	if not exist $@ ren txt.tmp $@
#	touch $@
#	attrib -a $@
#
#.mm.mak:
#	emacs -batch -l mymac -f mymac-batch-expand-files $< mf.tmp
#	sed -e "/^# !DIST\.$/,/^# YOW\.$/s/^/#/" -e "s/^## YOW\.$/# YOW./" mf.tmp > $@
#	del mf.tmp
#
#c0s.obj: c0.asm
#	tasm /M /MX /T /D__SMALL__ /D_DSSTACK_ c0, c0s
#
#xdist:
#	-del mysc.zip
#	pkzip -p -ex mysc.zip *.h *.c *.cs makefile *.mm mymac*.el 
#
#install: $(CMDS)
#	for %i in ($**) do tdstrip %i D:\BIN\%i
#
#dist:
#	-del myscdist.zip
#	-md dist
#	cd dist
#	$(MAKE) -f ../makefile -DSRCDIR=.. dist2 
#	cd ..
#
#dist2: readme.txt install.txt mf-msdos.mak mf-unix.mak cf-bcc.h cf-djgpp.h cf-sls.h cf-xenix.h $(SRCS) $(INCLUDES) $(XSRCS)
#	copy &&!
#s/ /\\
#/g
#! dist.sed
#	sed -f dist.sed <<!
#$**
#! > dist.lst
#	pkzip -p -ex $(SRCDIR)/myscdist.zip @dist.lst
#
#mf: makefile.new
#	$(MAKE) -DBUILD_MAKEFILE -f makefile.new makefile gcc.cfl
#
#!ifdef BUILD_MAKEFILE
#
#MAKEFILE = makefile.new
#
#makefile: makefile.new mf-deps.mak
#	copy makefile makefile.bak
#	copy makefile.new+mf-deps.mak makefile
#
#mf-deps.mak: $(SRCS) $(XSRCS) $(INCLUDES)
#	$(GCC) -MM @&&:
#$(GCFLAGS) -MM $(SRCS) $(XSRCS)
#: > mf-deps.tmp
#	sed -e "s/\.cs\.o/.x/" -e "s/ :/:/" -e "s/ cf-djgpp\.h//" mf-deps.tmp > mf-deps.mak
#	del mf-deps.tmp
#
#!endif
#
#$(XSRCS): make-cs.tmp
#
#make-cs.tmp: make-cs.mm mf-inc.mm
#	emacs -batch -l mymac -f mymac-batch-expand-files \
#		make-cs.mm make-cs.tmp
#
#makefile.new: mf-msdos.mm mf-inc.mm
#	emacs -batch -l mymac -f mymac-batch-expand-files \
#		mf-msdos.mm makefile.new
#
#mf-msdos.mak: mf-msdos.mm mf-inc.mm
#mf-unix.mak: mf-unix.mm mf-unix.mm
#
## YOW. 
