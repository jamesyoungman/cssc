#! /bin/sh

# comment.sh:  Testing for correct expansion of formats for prs -d.

# Import common functions & definitions.
. test-common

expands_to () {
    # $1 -- label
    # $2 -- format
    # $3 -- expansion
docommand $1 "${prs} \"-d$2\" -r1.1 s.1" 0 "$3"
}

remove s.1 p.1 1 z.1

# Create file
docommand P1 "${admin} -n s.1" 0 "" ""

docommand P2 "${prs} -d':M:\n' s.1" 0 "1\n\n" ""

docommand P3 "${get} -e s.1" 0 "1.1\nnew delta 1.2\n0 lines\n" IGNORE
echo "hello from %M%" >> 1
docommand P4 "${delta} -y s.1" 0 "1.2\n1 inserted\n0 deleted\n0 unchanged\n" ""

expands_to X1  :I:      "1.1\n"
expands_to X1r :R:      "1\n"
expands_to X1l :L:      "1\n"
expands_to X1b :B:      "\n"
expands_to X1s :S:      "\n"
expands_to X2  :BF:     "no\n"
expands_to X3  :DI:     "\n"
expands_to X4  :DL:     "00000/00000/00000\n"
expands_to X5  :DT:     "D\n"
expands_to X7  :J:      "no\n"
expands_to X8  :LK:     "none\n"
expands_to X9  :MF:     "no\n"
expands_to X10 :MP:     "none\n"
expands_to X11 :MR:     "\n"
expands_to X12 :Z:      '@(#)\n'
expands_to X13 'x\\ny'  "x\ny\n"
expands_to X14 ':Q:'    '\n'
expands_to X15 'x\ty'   'x\ty\n'
expands_to X16 'x\\ty'   'x\ty\n'
expands_to X17 'x\\ny'   'x\ny\n'

remove got.stdout expected.stdout
echo -n Z1...
${prs}  -d'\\' s.1 > got.stdout 2>got.stderr || fail prs failed.
echo \\            > expected.stdout || miscarry redirection to expected.stdout
diff expected.stdout got.stdout >/dev/null || fail stdout format error.
test -s got.stderr && fail expected empty stderr output
remove got.stderr got.stdout expected.stdout 
echo passed

remove s.1 p.1 z.1 1 command.log
success
