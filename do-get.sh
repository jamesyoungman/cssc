#! /bin/bash
for i in "admin.c
cdc.c
delta.c
file.c
fileiter.c
fileiter.h
filelock.h
get.c
l-split.c
mysc.h
mystring.c
mystring.h
pf-del.c
pfile.h
pipe.c
pipe.h
prs.c
run.c
run.h
sccsdate.c
sccsdate.h
sccsfile.c
sccsfile.h
sccsname.c
sccsname.h
sf-admin.c
sf-cdc.c
sf-chkid.c
sf-chkmr.h
sf-delta.c
sf-get.c
sf-get2.c
sf-prs.c
sf-write.c
split.c
unget.c" ; do co -l $i ; done
