#! /bin/sh

# Test for empty v (MR validation) flag.

. ../common/test-common

g=empty_mr_validator
s="s.${g}"
p="p.${g}"
z="s.${g}"

remove "${g}" "${s}" "${z}" "${p}"
copy setup_s_file s.empty_mr_validator.input  s.empty_mr_validator

# Key point of this test is that none of the commands should crash.
docommand E1 "${vg_get} -e ${s}" 0 IGNORE IGNORE

remove "${g}" "${s}" "${z}" "${p}"
success
