#
# Makefile
#
# By Ross Ridge
# Public Domain
#
# Makefile for compiling the MySC utilities under Unix.
#
# @(#) MySC mf-unix.mm 1.6 93/12/30 19:48:48
# @(#) MySC mf-inc.mm 1.4 93/11/09 23:15:38
#

SHELL = /bin/sh

CC = gcc
CFLAGS = -Wall -Wpointer-arith -Wwrite-strings -Wmissing-prototypes\
	 -Wconversion -Wcast-align -Wstrict-prototypes -Wnested-externs\
	 -Winline -Wno-comment -x c++ -fno-omit-frame-pointer -g -pipe
LFLAGS = -g
LIBS =

CMDS = get delta admin prs what unget sact cdc rmdel
XCMDS = get.x delta.x admin.x prs.x what.x unget.x sact.x cdc.x rmdel.x

GETOBJS = get.o quit.o xalloc.o mystring.o sccsname.o sid.o sccsdate.o \
	linebuf.o file.o split.o getopt.o fileiter.o sccsfile.o sf-get.o \
	sf-get2.o sf-get3.o sf-chkid.o pfile.o pf-add.o
DELTAOBJS = delta.o quit.o xalloc.o mystring.o sccsname.o sid.o sccsdate.o \
	linebuf.o file.o split.o getopt.o fileiter.o sccsfile.o sf-delta.o \
	sf-get.o sf-get3.o sf-chkid.o sf-write.o sf-add.o pfile.o pf-del.o \
	pipe.o run.o l-split.o prompt.o
ADMINOBJS = admin.o quit.o xalloc.o mystring.o sccsname.o sid.o sccsdate.o \
	linebuf.o file.o split.o getopt.o fileiter.o sccsfile.o sf-admin.o \
	sf-chkid.o sf-write.o sf-add.o run.o l-split.o prompt.o
PRSOBJS = prs.o quit.o xalloc.o mystring.o sccsname.o sid.o sccsdate.o \
	linebuf.o file.o split.o getopt.o fileiter.o sccsfile.o sf-prs.o \
	sf-get.o sf-chkid.o
WHATOBJS = what.o quit.o getopt.o
UNGETOBJS = unget.o quit.o xalloc.o mystring.o sccsname.o sid.o sccsdate.o \
	linebuf.o file.o split.o getopt.o fileiter.o pfile.o pf-del.o
SACTOBJS = sact.o quit.o xalloc.o mystring.o sccsname.o sid.o sccsdate.o \
	linebuf.o file.o split.o getopt.o fileiter.o pfile.o
CDCOBJS = cdc.o quit.o xalloc.o mystring.o sccsname.o sid.o sccsdate.o \
	linebuf.o file.o split.o getopt.o fileiter.o sccsfile.o sf-cdc.o \
	sf-write.o run.o l-split.o prompt.o
RMDELOBJS = rmdel.o quit.o xalloc.o mystring.o sccsname.o sid.o sccsdate.o \
	linebuf.o file.o split.o getopt.o fileiter.o sccsfile.o sf-rmdel.o \
	sf-write.o pfile.o
SRCS = admin.c cdc.c delta.c file.c fileiter.c get.c getopt.c l-split.c \
	linebuf.c mystring.c pf-add.c pf-del.c pfile.c pipe.c prompt.c prs.c \
	quit.c rmdel.c run.c sact.c sccsdate.c sccsfile.c sccsname.c \
	sf-add.c sf-admin.c sf-cdc.c sf-chkid.c sf-delta.c sf-get.c \
	sf-get2.c sf-get3.c sf-prs.c sf-rmdel.c sf-write.c sid.c split.c \
	unget.c what.c xalloc.c
XSRCS = $(XCMDS:.x=.cs)
INCLUDES = defaults.h file.h fileiter.h filelock.h getopt.h linebuf.h \
	list.h mysc.h mystring.h pfile.h pipe.h quit.h run.h sccsdate.h \
	sccsfile.h sccsname.h seqstate.h sf-chkmr.h sid.h sid_list.h stack.h \
	sysdep.h xalloc.h fsync.c _chmod.c strstr.c list.c sid_list.c \
	sl-merge.c

.SUFFIXES: .x .cs

.c.o: 
	$(CC) $(CFLAGS) -c $<

.c.s:
	$(CC) $(CFLAGS) -S $<

.c.i:
	$(CC) $(CFLAGS) -E $< > $@

.cs.x:
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $< $(LIBS)

it: $(CMDS)

xgcc: $(XCMDS)

get: $(GETOBJS)
	$(CC) $(LFLAGS) -o get $(GETOBJS) $(LIBS)

delta: $(DELTAOBJS)
	$(CC) $(LFLAGS) -o delta $(DELTAOBJS) $(LIBS)

admin: $(ADMINOBJS)
	$(CC) $(LFLAGS) -o admin $(ADMINOBJS) $(LIBS)

prs: $(PRSOBJS)
	$(CC) $(LFLAGS) -o prs $(PRSOBJS) $(LIBS)

what: $(WHATOBJS)
	$(CC) $(LFLAGS) -o what $(WHATOBJS) $(LIBS)

unget: $(UNGETOBJS)
	$(CC) $(LFLAGS) -o unget $(UNGETOBJS) $(LIBS)

sact: $(SACTOBJS)
	$(CC) $(LFLAGS) -o sact $(SACTOBJS) $(LIBS)

cdc: $(CDCOBJS)
	$(CC) $(LFLAGS) -o cdc $(CDCOBJS) $(LIBS)

rmdel: $(RMDELOBJS)
	$(CC) $(LFLAGS) -o rmdel $(RMDELOBJS) $(LIBS)


sl-test: sid.h sid.c sid_list.h sid_list.c
	$(CC) $(CFLAGS) $(LFLAGS) -DTEST -o sl-test sid_list.c

gtest:
	-rm -f test/[spx].passwd
	./admin -itest/passwd.1 test/s.passwd
	./get -e -g test/s.passwd
	cp test/passwd.2 passwd
	./delta -Y test/s.passwd
	./get -e -g test/s.passwd
	cp test/passwd.3 passwd
	./delta -Y test/s.passwd
	./get -e -g -x1.2 test/s.passwd
	cp test/passwd.4 passwd
	./delta -Y test/s.passwd
	./get -e -g test/s.passwd
	cp test/passwd.5 passwd
	./delta -Y test/s.passwd
	./get -e -g -r1.3 test/s.passwd
	cp test/passwd.6 passwd
	./delta -Y test/s.passwd
	./get -r1.1 -p test/s.passwd > test/passwd.m1
	./get -r1.2 -p test/s.passwd > test/passwd.m2
	./get -r1.3 -p test/s.passwd > test/passwd.m3
	./get -r1.4 -p test/s.passwd > test/passwd.m4
	./get -r1.5 -p test/s.passwd > test/passwd.m5
	./get -r1.3.1.1 -p test/s.passwd > test/passwd.m6

depend: $(SRCS) $(XSRCS) $(INCLUDES) config.h
	sed -e '/^# YOW.$$/p' -e '/^# YOW.$$/,$$d' Makefile > makefile.new
	gcc $(CFLAGS) -MM $(SRCS) $(XSRCS) | sed -e "s/\.cs\.o/.x/" -e "s/ :/:/" -e "s/ cf-[^.]*\.h//" >> makefile.new

clean:
	-rm -f $(CMDS) $(XCMDS) *.o 
	-rm -f core

## !DIST.
#
#D = .
#
#.mm.mak:
#	emacs -batch -l mymac -f mymac-batch-expand-files \
#		$< mf.tmp
#	sed -e '/^# !DIST\.$$/,/^# YOW\.$$/s/^/#/' -e 's/^## YOW\.$$/# YOW./' mf.tmp > $@
#	rm mf.tmp
#
#dist: mysc.tar.Z mysc.shar.01
#
#Dist.stamp: myscdist.zip
#	-rm -rf Dist
#	mkdir Dist
#	cd Dist; unzip -a -d ../myscdist.zip
#	mv Dist/readme.txt Dist/README
#	mv Dist/install.txt Dist/INSTALL
#	chmod 644 Dist/*
#	touch Dist.stamp
#
#mysc.tar.Z: Dist.stamp
#	cd Dist; tar cf ../mysc.tar README INSTALL mf-msdos.mak mf-unix.mak cf-bcc.h cf-djgpp.h cf-sls.h cf-xenix.h $(SRCS) $(INCLUDES) $(XSRCS)
#	-rm -f mysc.tar.Z
#	compress mysc.tar
#
#mysc.shar.01: Dist.stamp
#	cd Dist; shar -a -s "ross@utopia.druid.com" -n mysc -o ../mysc.shar -l 45 README INSTALL mf-msdos.mak mf-unix.mak cf-bcc.h cf-djgpp.h cf-sls.h cf-xenix.h $(SRCS) $(INCLUDES) $(XSRCS) 
#
#mf: makefile.new
#	$(MAKE) -f makefile.new MAKEFILE=Makefile Makefile
#
#MAKEFILE = GNU-make-botch
#
#$(MAKEFILE): makefile.new mf-deps.mak
#	cp Makefile Makefile.bak
#	cat makefile.new mf-deps.mak > Makefile
#
#mf-deps.mak: $(SRCS) $(XSRCS) $(INCLUDES)
#	gcc $(CFLAGS) -MM $(SRCS) $(XSRCS) | sed -e "s/\.cs\.o/.x/" -e "s/ :/:/" -e "s/ cf-[^.]*\.h//" > mf-deps.mak
#	
#$(XSRCS): make-cs.tmp
#
#make-cs.tmp: make-cs.mm mf-inc.mm
#	emacs -batch -l mymac -f mymac-batch-expand-files \
#		make-cs.mm make-cs.tmp
#
#makefile.new: mf-unix.mm mf-inc.mm
#	emacs -batch -l mymac -f mymac-batch-expand-files \
#		mf-unix.mm makefile.new
#
#mf-msdos.mak: mf-msdos.mm mf-inc.mm
#mf-unix.mak: mf-unix.mm mf-unix.mm
#
# YOW.
