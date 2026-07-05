#! /bin/sh

# exists.sh:  What if the input file doesn't exist?

# Import common functions & definitions.
. ../common/test-common.sh

g1=new1.txt
g2=new2.txt
s1="s.${g1}"
s2="s.${g2}"
p1="p.${g1}"
p2="p.${g2}"
x1="x.${g1}"
x2="x.${g2}"
m=mfile

cleanup() {
    remove "${s1}" "${g1}" "${p1}" "${s2}" "${g2}" "${p2}"
    rmove "old.${g1}" "old.${g2}" "${x1}" "${x2}"
    remove "${m}"
    remove xxx1 xxx2
}

cleanup


echo "%M%" >"${m}" || abandon_test_script "could not create ${m}."

rm -f "${p1}" || abandon_test_script "could not remove ${p1}"
ln -s / "${p1}" || abandon_test_script "could not ln -s / ${p1}"
docommand e1 "${vg_unget} -r1.2 ${s1}" 1 "IGNORE" "IGNORE"
remove "${p1}"


docommand e2 "${admin} -i ${s1}" 0 "" "" <"${m}"
docommand e3 "${get} -e ${s1}" 0 IGNORE IGNORE
docommand e4 "${vg_unget} -r1.2 ${s1}" 0 "IGNORE" "IGNORE"

cleanup
success
