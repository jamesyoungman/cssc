#! /bin/sh
# driver-basic.sh:  Testing for the basic operation of the BSD wrapper "sccs".
#                   We test each of the subcommands.

# Import common functions & definitions.
. ../common/test-common



# Find the sccs driver program.   It's not done in 
# tests/common/command-names because we have to run the candidate 
# to find out if it accepts the --prefix option.
#
if test x${sccs} = x
then
    if test -f ${dir}/sccs
    then
	sccsprog="${dir}/sccs"
    else
	case ${dir} in 
	    ../..) sccsprog="${dir}/bsd/sccs"
		    ;;
	    *) sccsprog="sccs"
		    ;;
	esac
    fi

    # Find out if it takes the --prefix option.  If so,
    # use it.
    if ${sccsprog} --cssc >/dev/null 2>&1
    then
	sccsargs="--prefix=${dir}/"
    else
	sccsargs=""
    fi

    sccs="${sccsprog} ${sccsargs}"
fi


remove command.log log log.stdout log.stderr SCCS
mkdir SCCS 2>/dev/null

g=tfile 
s=SCCS/s.${g} 
p=SCCS/p.${g} 
x=SCCS/x.${g} 
z=SCCS/z.${g}
remove $s $p $g $x $z

echo "Using the driver program ${sccs}"


# Create the input file.
cat > $g <<EOF
%M%: This is a test file containing nothing interesting.
EOF

#
# Creating the s-file. 
#
# Create the s-file the traditional way...
docommand a1 "${sccs} admin -i$g $s" 0 \
    ""                                              IGNORE
docommand a2 "test -f $s" 0 "" ""
remove $s

docommand a3 "${sccs} enter $g" 0 \
    "\n$g:\n"                                        IGNORE
docommand a4 "test -f $s"  0 "" ""

# Check the backup file still exists.
docommand a5 "test -f ,$g" 0 "" ""
remove ,$g

#
# Making deltas.
#

# First the traditional way.
docommand b1 "${sccs} get -e $s" 0 \
    "1.1\nnew delta 1.2\n1 lines\n"                 IGNORE

echo "hello" >>$g
docommand b2 "${sccs} delta -y\"\" $s" 0 \
    "1.2\n1 inserted\n0 deleted\n1 unchanged\n"     IGNORE


# Now with edit and delget.    
docommand b3 "${sccs} edit $s"  0 \
    "1.2\nnew delta 1.3\n2 lines\n"                 IGNORE


echo "there" >>$g
docommand b4 "${sccs} deledit -y'' $s" IGNORE \
 "1.3\n1 inserted\n0 deleted\n2 unchanged\n1.3\nnew delta 1.4\n" \
 IGNORE
# g-file should now exist and be writable.
docommand b5 "test -w $g" 0 "" ""


echo '%A%' >>$g
docommand b6 "${sccs} delget -y'' $s" 0 \
 "1.4\n1 inserted\n0 deleted\n3 unchanged\n1.4\n4 lines\n" \
 IGNORE
# g-file should now exist but not be writable.
docommand b7 "test -w $g" 1 "" ""
docommand b8 "test -f $g" 0 "" ""



#
# fix
#
docommand c1 "${sccs} fix -r1.4 $s" 0 \
 "1.4\n4 lines\n1.3\nnew delta 1.4\n" \
 IGNORE

docommand c2 "${sccs} tell" 0 "tfile\n" ""

docommand c3 "${sccs} delget -y'' $s" 0 \
 "1.4\n1 inserted\n0 deleted\n3 unchanged\n1.4\n4 lines\n" \
 IGNORE


#
# rmdel
#
# Make sure rmdel on its own works OK.
docommand d1 "${sccs} rmdel -r1.4 $s" 0 "" ""

# Make sure that revision is not still present.
docommand d2 "${sccs} get -p -r1.4 $s" 1 "" IGNORE

# Make sure that previous revision is still present.
docommand d3 "${sccs} get -p -r1.3 $s" 0 IGNORE "1.3\n3 lines\n"


#
# what
#
docommand e1 "${sccs} what $g" 0 "${g}:\n\t ${g} 1.4@(#)\n" ""


# 
# enter
# 
remove "foo" ",foo" "SCCS/s.foo"
echo "%Z%" >foo
docommand f1 "test -f ,foo" 1 "" ""
docommand f2 "${sccs} enter foo" 0 "\nfoo:\n" ""
docommand f3 "test -f ,foo" 0 "" ""
docommand f4 "test -f SCCS/s.foo" 0 "" ""
remove ",foo"

#
# clean
# 
docommand g1 "${sccs} edit SCCS/s.foo" 0 \
				    "1.1\nnew delta 1.2\n1 lines\n" ""

# Make sure foo and tfile exist but only foo is writable.
docommand g2 "test -f foo"   0 "" ""
docommand g3 "test -f tfile" 0 "" ""
docommand g4 "test -w foo"   0 "" ""
docommand g5 "test -w tfile" 1 "" ""
docommand g6 "${sccs} clean" 0 IGNORE ""
# Make sure tfile is now gone and foo is not.
docommand g7 "test -f tfile" 1 "" ""
docommand g8 "test -f foo"   0 "" ""
docommand g9 "test -w foo"   0 "" ""

#
# unedit 
#
docommand h1 "${sccs} unedit foo" 0 \
 "1.1\n1 lines\n         foo: removed\n" ""
# That's 9 spaces.


#
# info
#
docommand i1 "${sccs} info -b" 0 "Nothing being edited (on trunk)\n" ""
docommand i2 "${sccs} info"    0 "Nothing being edited\n" ""
remove SCCS/s.foo foo


#
# check
#
docommand j1 "${sccs} check" 0 "" ""
docommand j2 "${sccs} edit $s" 0 IGNORE IGNORE
docommand j3 "${sccs} check" 1 IGNORE ""
docommand j4 "${sccs} unedit $g" 0 IGNORE IGNORE



remove {expected,got}.std{out,err} last.command 
remove $s $p $g $x $z
success

#
# Still need to test:-

# cdc, comb, help, prs, prt, val, sccsdiff, diffs, -diff,
# branch, create

#
# Tests that would need a canned SCCS file:-
#
# print, info

	    
docommand B6 "${get} -e $s" 0 \
    "1.3\nnew delta 1.4\n2 lines\n"                 IGNORE
cp test/passwd.4 passwd
docommand B7 "${delta} -y'' $s" 0 \
    "1.4\n1 inserted\n1 deleted\n1 unchanged\n"     IGNORE
docommand B8 "${get} -e $s" 0 \
    "1.4\nnew delta 1.5\n2 lines\n"                 IGNORE
cp test/passwd.5 passwd
docommand B9 "${delta} -y'' $s" 0 \
    "1.5\n1 inserted\n1 deleted\n1 unchanged\n"     IGNORE
docommand B10 "${get} -e -r1.3 $s" 0 \
    "1.3\nnew delta 1.3.1.1\n2 lines\n"             IGNORE
cp test/passwd.6 passwd
docommand B11 "${delta} -y'' $s" 0 \
    "1.3.1.1\n1 inserted\n1 deleted\n1 unchanged\n" IGNORE

rm -rf test
remove passwd command.log $s $g $x $z $p SCCS
success

